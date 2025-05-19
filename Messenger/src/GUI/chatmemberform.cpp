#include "../../include/GUI/chatmemberform.h"
#include "ui_chatmemberform.h"

ChatMemberForm::ChatMemberForm(bool add, std::shared_ptr<Contact> contact, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatMemberForm),
    add_(add), contact_(contact)
{
    ui->setupUi(this);

    // ui->picture_label->setPixmap(QPixmap(QString::fromStdString(contact->picture)));
    ui->nickname_label->setText(QString::fromStdString(contact->name));
    ui->pushButton->setText(add_ ? "v" : "x");
}

ChatMemberForm::~ChatMemberForm()
{
    delete ui;
}

int ChatMemberForm::get_id() const
{
    return contact_->id;
}

std::shared_ptr<Contact> ChatMemberForm::get_contact() const
{
    return contact_;
}

void ChatMemberForm::set_add(bool add)
{
    add_ = add;
    ui->pushButton->setText(add_ ? "v" : "x");
}

void ChatMemberForm::on_pushButton_clicked()
{
    emit move_form(contact_->id, add_ ? true : false);
}
