#ifndef CHATMEMBERFORM_H
#define CHATMEMBERFORM_H

#include "../../../API/common_structure.h"

#include <memory>
#include <QWidget>

namespace Ui {
class ChatMemberForm;
}

class ChatMemberForm : public QWidget
{
    Q_OBJECT

public:

    explicit ChatMemberForm(bool add, std::shared_ptr<Contact> contact, QWidget *parent = nullptr);

    ~ChatMemberForm();

    int get_id() const;

    std::shared_ptr<Contact> get_contact() const;

    void set_add(bool add);

private slots:

    void on_pushButton_clicked();

signals:

    void move_form(int id, bool add);

private:

    Ui::ChatMemberForm *ui;

    bool add_;

    std::shared_ptr<Contact> contact_;
};

#endif // CHATMEMBERFORM_H
