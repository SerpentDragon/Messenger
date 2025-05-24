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
}

ChatParamsWindow::~ChatParamsWindow()
{
    delete ui;
}

void ChatParamsWindow::display_contacts(const std::vector<Contact>& contacts)
{
    for(const auto& contact : contacts)
    {
        if (contact.chat == -1)
        {
            auto cmf = std::shared_ptr<ChatMemberForm>(
                new ChatMemberForm(true, std::make_shared<Contact>(contact), this));

            contacts_.emplace_back(cmf);

            QObject::connect(&*cmf, &ChatMemberForm::move_form, this, &ChatParamsWindow::move_form);
        }
    }

    QStandardItemModel* model = new QStandardItemModel(this);
    ui->contacts_list->setModel(model);

    for(int i = 0; i < contacts_.size(); i++)
    {
        QStandardItem* item = new QStandardItem;
        item->setSizeHint(contacts_[i]->sizeHint());
        model->appendRow(item);

        QModelIndex index = model->indexFromItem(item);
        ui->contacts_list->setIndexWidget(index, &*contacts_[i]);
    }
}

void ChatParamsWindow::move_form(int id, bool add)
{
    auto& from_vec = add ? contacts_ : members_;
    auto& to_vec = add ? members_ : contacts_;
    QListView* from_list = add ? ui->contacts_list : ui->members_list;
    QListView* to_list   = add ? ui->members_list : ui->contacts_list;

    delete_widget_from_list(from_list, id);

    auto it = std::find_if(from_vec.begin(), from_vec.end(),
                           [id](const std::shared_ptr<ChatMemberForm>& widget)
                           {
                               return widget->get_id() == id;
                           });

    if (it == from_vec.end()) return;

    auto contact = (*it)->get_contact();
    auto form = std::shared_ptr<ChatMemberForm>(
        new ChatMemberForm(add ? false : true, contact, this));
    QObject::connect(&*form, &ChatMemberForm::move_form, this, &ChatParamsWindow::move_form);

    from_vec.erase(it);
    to_vec.push_back(form);

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(to_list->model());
    if (!model)
    {
        model = new QStandardItemModel(this);
        to_list->setModel(model);
    }

    QStandardItem* item = new QStandardItem();
    item->setSizeHint(form->sizeHint());
    model->appendRow(item);

    QModelIndex index = model->indexFromItem(item);
    to_list->setIndexWidget(index, &*form);
}

void ChatParamsWindow::delete_widget_from_list(QListView* list, int id)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(list->model());

    if (!model) return;

    for (int row = 0; row < model->rowCount(); ++row)
    {
        QModelIndex index = model->index(row, 0);
        QWidget* index_widget = list->indexWidget(index);

        ChatMemberForm* form = qobject_cast<ChatMemberForm*>(index_widget);
        if (!form) continue;

        if (form->get_id() == id)
        {
            model->removeRow(row);
            break;
        }
    }
}

void ChatParamsWindow::on_OK_button_pressed()
{
    QString chat_name = ui->chat_name_edit->text();
    if (chat_name.size() == 0)
    {
        QMessageBox::critical(this, "Error", "Chat must have a name");
        return;
    }

    bool self_destructive = ui->self_dest_check_box->isChecked();
    QDateTime date_time;

    if(self_destructive)
    {
        date_time = ui->dateTimeEdit->dateTime();

        // if(date_time < QDateTime::currentDateTime().addSecs(15 * 60))
        // {
        //     QMessageBox::critical(this, "Error", "No less then 15 min");
        //     return;
        // }
    }

    qint64 unix_time = date_time.isValid() ? date_time.toSecsSinceEpoch() : -1;

    std::vector<int> chat_members(members_.size());
    if (members_.size() == 0)
    {
        QMessageBox::critical(this, "Error", "You must add at least 1 member");
        return;
    }

    for(const auto& mem : members_)
    {
        qDebug() << "PUSH MEMBER BACK: " << mem->get_id() << '\n';
        chat_members.push_back(mem->get_id());
    }

    qDebug() << "UNIX TIME:" << unix_time;

    emit create_new_chat(chat_name, unix_time, chat_members);

    this->close();
}

void ChatParamsWindow::on_CANCEL_button_pressed()
{
    ui->chat_name_edit->clear();
    ui->self_dest_check_box->setChecked(false);
    this->close();
}
