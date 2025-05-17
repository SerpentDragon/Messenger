#include "../../include/GUI/messageform.h"
#include "ui/ui_messageform.h"

#include <QDebug>

MessageForm::MessageForm(ClientMessage* message, bool left_align, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MessageForm), msg(message)
{
    ui->setupUi(this);

    ui->text_label->setText(QString::fromStdString(msg->text));
    ui->text_label->adjustSize();
    ui->text_label->updateGeometry();

    ui->text_label->parentWidget()->updateGeometry();
    ui->text_label->parentWidget()->adjustSize();

    ui->nickname_label->setText(QString::fromStdString(msg->sender));
    ui->time_label->setText(QString::fromStdString(msg->time));

    Qt::Alignment text_align = Qt::AlignVCenter;

    if (left_align)
    {
        ui->my_picture_label->hide();
        // ui->picture_label->setPixmap();
        text_align |= Qt::AlignLeft;
    }
    else
    {
        ui->picture_label->hide();
        // ui->my_picture_label->setPixmap();
        text_align |= Qt::AlignRight;
    }

    ui->nickname_label->setAlignment(text_align);
    ui->text_label->setAlignment(text_align);
    ui->time_label->setAlignment(text_align);

    // qDebug() << this->size() << '\n';
    // qDebug() << ui->text_label->size() << '\n';
}

MessageForm::~MessageForm()
{
    delete ui;
}
