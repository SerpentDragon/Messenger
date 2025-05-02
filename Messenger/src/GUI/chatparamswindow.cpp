#include "../../include/GUI/chatparamswindow.h"
#include "ui/ui_chatparamswindow.h"

ChatParamsWindow::ChatParamsWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatParamsWindow)
{
    ui->setupUi(this);

    QWidget* p = this->parentWidget();
    if (p != nullptr)
    {
        this->move((p->width() - this->width()) / 2,
                   (p->height() - this->height()) / 2);
    }

    ui->confirm_chat_name_button->setIcon(QIcon(":main_window/main_window/check.png"));
}

ChatParamsWindow::~ChatParamsWindow()
{
    delete ui;
}

void ChatParamsWindow::on_OK_button_pressed()
{
    this->close();
}


void ChatParamsWindow::on_CANCEL_button_pressed()
{
    this->close();
}
