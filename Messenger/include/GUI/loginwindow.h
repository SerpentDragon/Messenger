#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include "../Client/client.h"
#include "common.h"

#include <QMainWindow>
#include <QMessageBox>
#include <QStyle>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit LoginWindow(QWidget* parent = nullptr);

    ~LoginWindow();

private:

    void setup_icons();

public slots:

    void process_auth_resp(SERVER_RESP_CODES resp, int id);

private slots:

    void on_sign_up_label_linkActivated(const QString& link);

    void on_log_in_button_pressed();

signals:

    void log_in_user(bool log_in, const std::string& nickname, const std::string& password);

    void log_in_success();

    void db_connect(bool log_in, int id, const std::string& nickname, const std::string& password);

private:

    Ui::LoginWindow* ui;

    enum class STATE : unsigned short
    {
        LOG_IN = 0,
        SIGN_UP = 1
    } state_;
};

#endif // LOGINWINDOW_H
