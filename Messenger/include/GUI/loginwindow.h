#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "../Client/client.h"
#include "common.h"

#include <QMainWindow>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit LoginWindow(Client& client, QWidget* parent = nullptr);

    ~LoginWindow();

signals:

    void log_in_success();

private:

    void setup_icons();

private slots:

    void on_sign_up_label_linkActivated(const QString& link);

    void on_log_in_button_pressed();

private:

    Ui::LoginWindow* ui;

    enum class STATE : unsigned short
    {
        LOG_IN = 0,
        SIGN_UP = 1
    } state;

    Client& client;
};

#endif // LOGINWINDOW_H
