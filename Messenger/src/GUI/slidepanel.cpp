#include "../../include/GUI/slidepanel.h"
#include "ui/ui_slidepanel.h"

SlidePanel::SlidePanel(int y, int width, int height, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SlidePanel)
    , y_(y), width_(width), height_(height)
{
    ui->setupUi(this);

    setFixedWidth(width_);
    setFixedHeight(height_);
    move(-width_, y_);

    setup_icons();
    hide_password_widgets();

    ui->prev_passwd->setEchoMode(QLineEdit::Password);
    ui->new_passwd->setEchoMode(QLineEdit::Password);
    ui->confirm_passwd->setEchoMode(QLineEdit::Password);

    ui->change_passwd_label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->change_passwd_label->setOpenExternalLinks(false);

    ui->change_nickname_button->setIcon(QIcon(":/main_window/resources/main_window/check.png"));
}

void SlidePanel::show_panel()
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(300);
    anim->setStartValue(pos());
    anim->setEndValue(QPoint(0, y_));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SlidePanel::hide_panel()
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(300);
    anim->setStartValue(pos());
    anim->setEndValue(QPoint(-width_, y_));
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SlidePanel::set_coordinates(int y, int width, int height)
{
    y_ = y;
    width_ = width;
    height_ = height;

    setFixedWidth(width_);
    setFixedHeight(height_);

    if (this->pos().x() < 0) this->move(-width_, y);

    this->move(this->pos().x(), y);
}

void SlidePanel::hide_password_widgets()
{
    ui->prev_passwd->hide();
    ui->new_passwd->hide();
    ui->confirm_passwd->hide();
    ui->change_passwd_button->hide();
}

void SlidePanel::setup_icons()
{
    IconPtr open = IconPtr(new QIcon(":/login_window/resources/login_window/opened_eye.png"));
    IconPtr close = IconPtr(new QIcon(":/login_window/resources/login_window/closed_eye.png"));

    create_password_toggle_action(ui->prev_passwd, close, open);
    create_password_toggle_action(ui->new_passwd, close, open);
    create_password_toggle_action(ui->confirm_passwd, close, open);
}

void SlidePanel::on_change_passwd_label_linkActivated(const QString &link)
{
    static bool show = false;

    if (show == false)
    {
        ui->prev_passwd->show();
        ui->new_passwd->show();
        ui->confirm_passwd->show();
        ui->change_passwd_button->show();
    }
    else
    {
        hide_password_widgets();
    }

    show = !show;
}

