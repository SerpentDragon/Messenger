#include "../include/GUI/mainwindow.h"
#include "../include/GUI/loginwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QStackedWidget>

class ClientApplication
{
public:

    ClientApplication(boost::asio::io_service& io) : io_(io)
    {
        client_ = std::make_unique<Client>(io_, "127.0.0.1", 1545);
        //client_->connect();

        login_window_ = std::make_unique<LoginWindow>(*client_);
        main_window_ = std::make_unique<MainWindow>();

        create_window_set();
    }

private:

    void create_window_set()
    {
        stkw_.addWidget(&*login_window_);
        stkw_.addWidget(&*main_window_);
        // stkw_.setCurrentWidget(&*login_window_);
        stkw_.setCurrentWidget(&*main_window_);

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
                stkw_.setFixedSize(current->size());
            }
        });

        stkw_.show();
    }

private:

    boost::asio::io_service& io_;
    std::unique_ptr<Client> client_;

    QStackedWidget stkw_;
    std::unique_ptr<LoginWindow> login_window_;
    std::unique_ptr<MainWindow> main_window_;
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
