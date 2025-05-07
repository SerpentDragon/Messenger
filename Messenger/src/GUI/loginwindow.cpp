#include "../../include/GUI/loginwindow.h"
#include "ui/ui_loginwindow.h"

const QString htmlTemplate = R"(
    <a href="my_click">
        <span style="color:blue; text-decoration:underline;">%1</span>
    </a>
)";

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow),
    state_(STATE::LOG_IN)
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

void LoginWindow::setup_icons()
{
    IconPtr open = IconPtr(new QIcon(":/login_window/login_window/opened_eye.png"));
    IconPtr close = IconPtr(new QIcon(":/login_window/login_window/closed_eye.png"));

    create_password_toggle_action(ui->password_edit, close, open);
    create_password_toggle_action(ui->repeat_password_edit, close, open);
}

void LoginWindow::process_auth_resp(SERVER_RESP_CODES resp, int id)
{
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

    emit log_in_success();
}

void LoginWindow::on_sign_up_label_linkActivated(const QString &link)
{
    ui->repeat_password_edit->clear();

    if (state_ == STATE::LOG_IN)
    {
        ui->log_in_label->setText("Sign Up");
        ui->repeat_password_label->show();
        ui->repeat_password_edit->show();
        ui->sign_up_label->setText(htmlTemplate.arg("Back"));
        ui->log_in_button->setText("Sign Up");

        state_ = STATE::SIGN_UP;
    }
    else if (state_ == STATE::SIGN_UP)
    {
        ui->log_in_label->setText("Log In");
        ui->repeat_password_label->hide();
        ui->repeat_password_edit->hide();
        ui->sign_up_label->setText(htmlTemplate.arg("Sign Up?"));
        ui->log_in_button->setText("Log In");

        state_ = STATE::LOG_IN;
    }
}

void LoginWindow::on_log_in_button_pressed()
{
    bool log_in = true;
    const std::string nickname = ui->nickname_edit->text().toStdString();
    const std::string password = ui->password_edit->text().toStdString();

    ui->nickname_edit->clear();
    ui->password_edit->clear();

    if (state_ == STATE::SIGN_UP)
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

    emit log_in_user(log_in, nickname, password);

    qDebug() << "EMITTED\n";
    
    // client_.send_user_data(log_in, nickname, password);
}
