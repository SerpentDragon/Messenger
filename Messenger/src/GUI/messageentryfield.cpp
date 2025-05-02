#include "../../include/GUI/messageentryfield.h"

MessageEntryField::MessageEntryField(QWidget *parent)
    : QWidget{parent}
{
    msg_edit_ = new AutoExpandTextEdit(this);

    msg_params_ = new QPushButton(this);
    attach_file_ = new QPushButton(this);
    send_msg_ = new QPushButton(this);

    set_buttons_style();
    msg_params_button_click();

    connect(msg_edit_, &AutoExpandTextEdit::change_height,
            this, &MessageEntryField::handle_height_change);
}

void MessageEntryField::showEvent(QShowEvent *event)
{
    calculate_widgets_pos();
}

void MessageEntryField::set_buttons_style()
{
    QString style = QString(R"(
        background-color: white;
        border: 1px solid #ccc;
        border-radius: %1px;
        color: black;)").arg(button_sz_ / 2);

    msg_params_->setStyleSheet(style);
    attach_file_->setStyleSheet(style);
    send_msg_->setStyleSheet(style);

    msg_params_->setIcon(QIcon(":/main_window/main_window/params.png"));
    attach_file_->setIcon(QIcon(":/main_window/main_window/clip.png"));
    send_msg_->setIcon(QIcon(":/main_window/main_window/plane.png"));

    send_msg_->setIconSize(QSize(30, 30));
}

void MessageEntryField::set_buttons_geometry()
{
    msg_params_->setGeometry(this->width() - 3 * button_sz_,
                             this->height() - button_sz_, button_sz_, button_sz_);
    attach_file_->setGeometry(this->width() - 2 * button_sz_,
                              this->height() - button_sz_, button_sz_, button_sz_);
    send_msg_->setGeometry(this->width() - button_sz_,
                           this->height() - button_sz_, button_sz_, button_sz_);
}

void MessageEntryField::msg_params_button_click()
{
    QMenu *menu = new QMenu(this->parentWidget());
    QWidgetAction* vanishing_msg_act = new QWidgetAction(menu);

    QCheckBox* vanishing_msg_chkbx = new QCheckBox("Vanishing message", menu);
    vanishing_msg_chkbx->setStyleSheet(R"(background-color: white;)");

    vanishing_msg_act->setDefaultWidget(vanishing_msg_chkbx);
    menu->addAction(vanishing_msg_act);

    connect(msg_params_, &QPushButton::clicked, this, [this, menu]() {
        QPoint point = msg_params_->mapToGlobal(QPoint{ 0, 0 });
        menu->popup({ point.x() - (menu->width() - msg_params_->width()) / 2 ,
                     point.y() - menu->height() });
    });
}

void MessageEntryField::calculate_widgets_pos()
{
    QWidget* p = this->parentWidget();

    setGeometry(0, p->height() - this->height(), p->width(), this->height());

    set_buttons_geometry();

    msg_edit_->set_width(this->width() - 3 * button_sz_);
}

void MessageEntryField::handle_height_change(int height)
{
    setGeometry(this->x(), this->parentWidget()->height() - height, this->width(), height);

    set_buttons_geometry();
}
