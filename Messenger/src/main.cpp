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
        client_ = std::make_unique<Client>(io_, "127.0.0.1", 1545);
        client_->connect();
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

        QObject::connect(&*main_window_, &MainWindow::set_contacts_from_db, this, &ClientApplication::set_contacts_from_db);
    }

    void db_connect(bool log_in, int id, const std::string& nickname)
    {
        qDebug() << "PREPARE TO CONNECT\n";

        db_manager_.db_connect(log_in, id, nickname);

        qDebug() << "CONNECTED\n";

        main_window_->set_contacts(db_manager_.get_contacts_list());

        qDebug() << "CONTACTS ARE SET\n";
    }

    void set_contacts_from_db()
    {
        main_window_->set_contacts(db_manager_.get_contacts_list());
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
