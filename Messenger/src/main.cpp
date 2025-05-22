#include "../include/GUI/mainwindow.h"
#include "../include/GUI/loginwindow.h"
#include "../include/Client/dbmanager.h"

#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QStackedWidget>

class ClientApplication : public QObject
{
public:

    ClientApplication(boost::asio::io_service& io) : io_(io)
    {
        Cryptographer::get_cryptographer()->generate_RSA_keys();
        auto keys = Cryptographer::get_cryptographer()->serialize_RSA_keys();

        client_ = std::make_unique<Client>(io_, "127.0.0.1", 1545);
        client_->connect();
        client_->send_public_key(keys.second);
        client_->start_read();

        login_window_ = std::make_unique<LoginWindow>();
        main_window_ = std::make_unique<MainWindow>();

        make_connections();

        create_window_set();
    }

private:

    void create_window_set()
    {
        stkw_.addWidget(&*login_window_);
        stkw_.addWidget(&*main_window_);
        stkw_.setCurrentWidget(&*login_window_);
        // stkw_.setCurrentWidget(&*main_window_);

        login_window_->setParent(&stkw_);
        main_window_->setParent(&stkw_);

        QObject::connect(&*login_window_, &LoginWindow::log_in_success, [&]() {
            stkw_.setCurrentWidget(&*main_window_);
        });

        QObject::connect(&stkw_, &QStackedWidget::currentChanged, [&](int index) {
            QWidget *current = stkw_.currentWidget();
            if (current)
            {
                stkw_.resize(current->sizeHint());
            }
        });

        stkw_.show();
    }

    void make_connections()
    {
        QObject::connect(&*login_window_, &LoginWindow::log_in_user, &*client_, &Client::log_in_user);
        QObject::connect(&*client_, &Client::auth_resp, &*login_window_, &LoginWindow::process_auth_resp);

        QObject::connect(&*login_window_, &LoginWindow::db_connect, this, &ClientApplication::db_connect);

        QObject::connect(&*main_window_, &MainWindow::send_message, &*client_, &Client::write);
        QObject::connect(&*client_, &Client::send_msg, &db_manager_, &DBManager::save_msg);
        QObject::connect(&*client_, &Client::receive_msg, &db_manager_, &DBManager::save_msg);
        QObject::connect(&db_manager_, &DBManager::receive_msg, &*main_window_, &MainWindow::receive_msg);
        QObject::connect(&db_manager_, &DBManager::display_sent_msg, &*main_window_, &MainWindow::display_sent_msg);

        QObject::connect(&*main_window_, &MainWindow::send_system_msg, &*client_, &Client::send_system_msg);
        QObject::connect(&*client_, &Client::list_of_contacts, &*main_window_, &MainWindow::list_of_contacts);

        QObject::connect(&*main_window_, &MainWindow::load_messages, &db_manager_, &DBManager::load_messages);
        QObject::connect(&db_manager_, &DBManager::loaded_messages, &*main_window_, &MainWindow::loaded_messages);

        QObject::connect(&*main_window_, &MainWindow::save_contact, &db_manager_, &DBManager::save_contact);
        QObject::connect(&*client_, &Client::save_contact, &db_manager_, &DBManager::save_contact);

        QObject::connect(&*main_window_, &MainWindow::set_contacts_from_db, this, &ClientApplication::set_contacts_from_db);

        QObject::connect(&db_manager_, &DBManager::update_keys_cash, &*client_, &Client::update_keys_cash);
        QObject::connect(&db_manager_, &DBManager::save_public_key, &*client_, &Client::save_public_key);

        QObject::connect(&*client_, &Client::add_contact, &*main_window_, &MainWindow::add_contact);

        QObject::connect(&*main_window_, &MainWindow::new_chat, &*client_, &Client::new_chat);
        QObject::connect(&*client_, &Client::add_new_chat, &db_manager_, &DBManager::save_chat);
        QObject::connect(&db_manager_, &DBManager::add_new_contact, &*main_window_, &MainWindow::add_contact);
    }

    void db_connect(bool log_in, int id, const std::string& nickname)
    {
        qDebug() << "PREPARE TO CONNECT\n";

        db_manager_.db_connect(log_in, id, nickname);

        std::pair<std::string,std::string> keys;

        if (log_in == false)
        {
            Cryptographer::get_cryptographer()->generate_RSA_keys();
            keys = Cryptographer::get_cryptographer()->serialize_RSA_keys();

            db_manager_.save_RSA_keys(keys); 
        }
        else
        {
            keys = db_manager_.load_RSA_keys();
            Cryptographer::get_cryptographer()->deserialize_RSA_keys(keys);

            qDebug() << "KEY FROM DB\n" << keys.second << "\n\n";
        }

        client_->send_system_msg(SYSTEM_MSG::LOAD_RSA_KEY, { QString::fromStdString(keys.second) });

        qDebug() << "CONNECTED\n";

        main_window_->set_contacts(db_manager_.get_contacts_list());
        main_window_->set_id(id);

        qDebug() << "CONTACTS ARE SET\n";
    }

    void set_contacts_from_db()
    {
        auto list = db_manager_.get_contacts_list();
        for(auto c : list) qDebug() << c.id << c.name;
        qDebug() << "ALLES\n";
        main_window_->set_contacts(list);
    }

private:

    boost::asio::io_service& io_;
    std::unique_ptr<Client> client_;

    QStackedWidget stkw_;
    std::unique_ptr<LoginWindow> login_window_;
    std::unique_ptr<MainWindow> main_window_;

    DBManager db_manager_;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try
    {
        boost::asio::io_service io;

        ClientApplication client_app(io);

        std::thread io_thread([&io]() {
            io.run();
        });

        int result = a.exec();

        io.stop();
        io_thread.join();

        return result;
    }
    catch(const std::exception& ex)
    {
        QMessageBox::critical(nullptr, "Error", ex.what());
        return 1;
    }
}
