#include "../../include/GUI/loginwindow.h"
#include "ui/ui_loginwindow.h"

const QString htmlTemplate = R"(
    <a href="my_click">
        <span style="color:blue; text-decoration:underline;">%1</span>
    </a>
)";

LoginWindow::LoginWindow(Client& cl, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow), client(cl),
    state(STATE::LOG_IN)
{
    ui->setupUi(this);

    ui->repeat_password_edit->hide();
    ui->repeat_password_label->hide();

    ui->password_edit->setEchoMode(QLineEdit::Password);
    ui->repeat_password_edit->setEchoMode(QLineEdit::Password);

    ui->sign_up_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->sign_up_label->setOpenExternalLinks(false);

    setup_icons();
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_sign_up_label_linkActivated(const QString &link)
{
    ui->repeat_password_edit->clear();

    if (state == STATE::LOG_IN)
    {
        ui->log_in_label->setText("Sign Up");
        ui->repeat_password_label->show();
        ui->repeat_password_edit->show();
        ui->sign_up_label->setText(htmlTemplate.arg("Back"));
        ui->log_in_button->setText("Sign Up");

        state = STATE::SIGN_UP;
    }
    else if (state == STATE::SIGN_UP)
    {
        ui->log_in_label->setText("Log In");
        ui->repeat_password_label->hide();
        ui->repeat_password_edit->hide();
        ui->sign_up_label->setText(htmlTemplate.arg("Sign Up?"));
        ui->log_in_button->setText("Log In");

        state = STATE::LOG_IN;
    }
}


void LoginWindow::setup_icons()
{
    IconPtr open = IconPtr(new QIcon(":/login_window/resources/login_window/opened_eye.png"));
    IconPtr close = IconPtr(new QIcon(":/login_window/resources/login_window/closed_eye.png"));

    create_password_toggle_action(ui->password_edit, close, open);
    create_password_toggle_action(ui->repeat_password_edit, close, open);
}

void LoginWindow::on_log_in_button_pressed()
{
    bool log_in = true;
    const std::string nickname = ui->nickname_edit->text().toStdString();
    const std::string password = ui->password_edit->text().toStdString();

    ui->nickname_edit->clear();
    ui->password_edit->clear();

    if (state == STATE::SIGN_UP)
    {
        const std::string repeated_password = ui->repeat_password_edit->text().toStdString();
        ui->repeat_password_edit->clear();

        if (password != repeated_password)
        {
            QMessageBox::critical(this, "Error", "Passwords don't match!");
            return;
        }

        log_in = false;
    }

    client.send_user_data(log_in, nickname, password);
    auto resp = client.get_auth_resp();

    if (resp != SERVER_RESP_CODES::OK)
    {
        QString msg;

        switch(resp)
        {
        case SERVER_RESP_CODES::ERROR:
            msg = "Error reading response from server!";
            break;
        case SERVER_RESP_CODES::NO_USER_NICKNAME:
            msg = "No user with that nickname!";
            break;
        case SERVER_RESP_CODES::WRONG_NICKNAME_PSSWD:
            msg = "Wronk nickname or password!";
            break;
        case SERVER_RESP_CODES::NICKNAME_TAKEN:
            msg = "Nickname is already taken!";
            break;
        default:
            msg = "Unknown error!";
            break;
        }

        QMessageBox::critical(this, "Error", msg);

        return;
    }

    // it's time to open mainwindow!!!
    // QMessageBox::information(this, "Info", "Success!");

    emit log_in_success();
}
