#include "../../include/GUI/mainwindow.h"
#include "ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_panel();
    setup_icons();

    msg_fld_ = new MessageEntryField(ui->messages_window);
    msg_fld_->show();
    msg_fld_->hide();

    connect(msg_fld_, &MessageEntryField::send_msg,
            this, &MainWindow::send_msg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_contacts(const std::vector<Contact> &contacts)
{
    qDebug() << __func__ << '\n';

    contacts_ = contacts;

    display_contacts();

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::set_id(const int id)
{
    my_id_ = id;
}

void MainWindow::setup_panel()
{
    qDebug() << __func__ << '\n';

    panel_ = new SlidePanel(ui->menubar->height() + ui->dialogs_list->y(),
                            ui->dialogs_list->width(), ui->dialogs_list->height(), this);
    panel_->show();

    connect(ui->menu_button, &QPushButton::clicked, [this]() {
        if (panel_->pos().x() < 0)
        {
            panel_->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
            panel_->show_panel();
        }
        else
            panel_->hide_panel();
    });
    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::setup_icons()
{
    qDebug() << __func__ << '\n';

    ui->menu_button->setIcon(QIcon(":/main_window/main_window/menu.png"));
    ui->new_chat_button->setIcon(QIcon(":/main_window/main_window/new_chat.png"));
    ui->chat_settings_button->setIcon(QIcon(":/main_window/main_window/params.png"));
    ui->P2P_button->setIcon(QIcon(":/main_window/main_window/P2P.png"));

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    qDebug() << __func__ << '\n';

    QWidget::resizeEvent(event);

    msg_fld_->calculate_widgets_pos();
    panel_->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());

    QAbstractItemModel* model = ui->messages_window->model();
    if (model == nullptr) return;

    for (int row = 0; row < model->rowCount(); ++row)
    {
        QModelIndex index = model->index(row, 0);
        QWidget *widget = ui->messages_window->indexWidget(index);
        if (widget)
        {
            widget->setFixedWidth(ui->messages_window->viewport()->width());
        }
    }

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::clear_list_view(QListView* lists)
{
    qDebug() << __func__ << '\n';

    QAbstractItemModel* model = lists->model();
    if (model == nullptr) return;

    int rowCount = model->rowCount();

    for (int row = 0; row < rowCount; ++row)
    {
        QModelIndex index = model->index(row, 0);
        QWidget* w = lists->indexWidget(index);
        if (w)
        {
            lists->setIndexWidget(index, nullptr);
            w->deleteLater();
        }
    }

    model->removeRows(0, rowCount);

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::display_contacts()
{
    qDebug() << __func__ << '\n';

    clear_list_view(ui->dialogs_list);

    QStandardItemModel* model = new QStandardItemModel(this);
    ui->dialogs_list->setModel(model);

    for(const auto& contact : contacts_)
    {
        QStandardItem* item = new QStandardItem;
        item->setSizeHint({ ui->dialogs_list->size().width(), 50 });
        model->appendRow(item);

        QModelIndex index = model->indexFromItem(item);

        QPixmap photo(QString::fromStdString(contact.picture));
        ContactEntry* ce = new ContactEntry(photo, QString::fromStdString(contact.name),
                                            contact.id, contact.chat);

        ui->dialogs_list->setIndexWidget(index, ce);

        // qDebug() << "CONTACT ENTRY: " << contact.id << ' ' << contact.name << ' ' << contact.chat << '\n';
    }

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::add_message(const ClientMessage& msg)
{
    qDebug() << __func__ << '\n';

    Contact* contact = get_contact_from_lst();
    if (contact == nullptr) return;

    messages_.push_back(msg);
    msg_forms_.push_back(std::make_shared<MessageForm>(
        &messages_.back(), messages_.back().sender_id != my_id_));

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->messages_window->model());
    if (model == nullptr)
    {
        model = new QStandardItemModel(this);
        ui->messages_window->setModel(model);
    }

    QStandardItem* item = new QStandardItem();
    item->setSizeHint(msg_forms_.back()->sizeHint());
    model->appendRow(item);

    QModelIndex index = model->index(model->rowCount() - 1, 0);
    ui->messages_window->setIndexWidget(index, &*msg_forms_.back());

    // qDebug() << msg_forms_.back()->msg->text << '\n';

    // if (messages_.size() > max_msg_count)
    // {
    //     messages_.pop_back();
    //     msg_forms_.pop_back();
    // }

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::display_messages()
{
    qDebug() << __func__ << '\n';

    clear_list_view(ui->messages_window);

    QStandardItemModel* model = new QStandardItemModel(this);
    ui->messages_window->setModel(model);

    for(int i = 0; i < messages_.size(); i++)
    {
        QStandardItem* item = new QStandardItem;
        item->setSizeHint(msg_forms_[i]->sizeHint());
        model->appendRow(item);

        QModelIndex index = model->indexFromItem(item);
        ui->messages_window->setIndexWidget(index, &*msg_forms_[i]);
    }

    ui->messages_window->scrollToBottom();

    qDebug() << "leave " << __func__ << '\n';
}

Contact* MainWindow::get_contact_from_lst()
{
    qDebug() << __func__ << '\n';

    QModelIndex index = ui->dialogs_list->currentIndex();
    int row = index.row();

    qDebug() << __func__ << " row = " << row << '\n';

    return row < 0 ? nullptr : &contacts_[row];
}

void MainWindow::send_msg(const QString& text)
{
    qDebug() << __func__ << '\n';

    Contact* contact = get_contact_from_lst();
    if (contact == nullptr) return;

    int client_id = contact->id;
    int chat_id = contact->chat;

    if (chat_id != -1)
    {
        for(int p : contact->participants) qDebug() << "PARTICIPANT:" << p;
    }

    qDebug() << client_id << ' ' << contact->name << ' ' << chat_id << '\n';

    SocketMessage msg
    {
        .receiver = chat_id == -1 ? std::vector{ client_id } : contact->participants,
        .text = text.toStdString(),
        .timestamp = generate_timestamp(),
        .chat = chat_id
    };

    emit send_message(msg);

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::receive_msg(const ClientMessage& msg)
{
    qDebug() << __func__ << '\n';

    qDebug() << "Prepare to display" << msg.sender_id << ' ' << msg.text << '\n';

    if (Contact* contact = get_contact_from_lst(); contact != nullptr)
    {
        qDebug() << msg.receiver_id << ' ' << contact->id << '\n';
        if (!msg_fld_->isHidden() && (msg.sender_id == contact->id) || msg.chat == contact->chat)
        {
            add_message(msg);
        }
    }

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::list_of_contacts(const std::string& name, const std::vector<Contact>& contacts_from_srv)
{
    qDebug() << __func__ << '\n';

    std::vector<Contact> all_contacts;

    for(const auto& cn : contacts_)
    {
        if (QString::fromStdString(cn.name).contains(QString::fromStdString(name)))
        {
            all_contacts.emplace_back(cn);
        }
    }

    std::copy_if(contacts_from_srv.begin(), contacts_from_srv.end(),
                 std::back_inserter(all_contacts),
                 [&](const Contact& cn)
                 {
                     // we should't display contacts we already have once again
                     return !std::any_of(contacts_.begin(), contacts_.end(), [&](const Contact& cnt){ return cnt.id == cn.id; });
                 });

    contacts_ = all_contacts;

    display_contacts();

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::loaded_messages(const std::deque<ClientMessage>& msgs)
{
    qDebug() << __func__ << '\n';

    Contact* contact = get_contact_from_lst();
    if (contact == nullptr) return;

    ui->dialog_name_label->setText(QString::fromStdString(contact->name));

    qDebug() << "SET NAME: " << contact->name << '\n';

    messages_ = msgs;
    msg_forms_.clear();

    for(int i = 0; i < max_msg_count && i < messages_.size(); i++)
    {
        msg_forms_.emplace_back(new MessageForm(&messages_[i], messages_[i].sender_id != my_id_));

        qDebug() << "ID: " << contact->id << '\n';
        qDebug() << "Sender: " << messages_[i].sender << ' ' << contact->name << ' ' << (messages_[i].sender != contact->name) << '\n';

        qDebug() << "MSG TEXT: " << messages_[i].text << '\n';
    }

    display_messages();

    qDebug() << "leave " << __func__ << '\n';
}

void MainWindow::display_sent_msg(const ClientMessage &msg)
{
    qDebug() << __func__ << '\n';
    add_message(msg);
    qDebug() << "leave" << __func__ << '\n';
}

void MainWindow::add_contact(const Contact& contact)
{
    qDebug() << __func__ << '\n';

    if (std::any_of(contacts_.begin(), contacts_.end(),
                    [contact](const Contact& cn){ return cn.id == contact.id; }))
    {
        return;
    }

    contacts_.push_back(contact);
    contacts_.back().saved_in_db = true;

    emit save_contact(contacts_.back());

    display_contacts();
    qDebug() << "leave" << __func__ << '\n';
}

void MainWindow::create_new_chat(const QString &name, qint64 time, const std::vector<int>& members)
{
    qDebug() << __func__ << '\n';
    qDebug() << "CREATE NEW CHAT\n";

    std::vector<int> mems;
    std::copy_if(members.begin(), members.end(), std::back_inserter(mems), [](int mem){ return mem > 0; });

    for(int i : mems) qDebug() << "CR NEW CHT: " << i << '\n';
    emit new_chat(name, time, mems);
    qDebug() << "leave" << __func__ << '\n';
}

void MainWindow::on_new_chat_button_pressed()
{
    chat_params_wnd_ = new ChatParamsWindow(this);
    QObject::connect(chat_params_wnd_, &ChatParamsWindow::create_new_chat,
                     this, &MainWindow::create_new_chat);

    chat_params_wnd_->display_contacts(contacts_);
    chat_params_wnd_->show();
}

void MainWindow::on_chat_settings_button_pressed()
{
    chat_params_wnd_ = new ChatParamsWindow(this);
    chat_params_wnd_->show();
}

void MainWindow::on_dialog_search_edit_returnPressed()
{
    qDebug() << __func__ << '\n';
    auto text = ui->dialog_search_edit->text();

    if (text.size() == 0)
    {
        qDebug() << "GET CONTACTS FROM DB\n";
        emit set_contacts_from_db();
    }
    else emit send_system_msg(SYSTEM_MSG::FIND_CONTACT, { text });
    qDebug() << "leave" << __func__ << '\n';
}

void MainWindow::on_dialogs_list_clicked(const QModelIndex& index)
{
    int row = index.row();

    Contact contact = contacts_[row];

    qDebug() << contact.id << ' ' << contact.name << ' ' << contact.chat << '\n';

    if (!contact.saved_in_db)
    {
        contact.saved_in_db = true;
        qDebug() << "save contact\n";
        emit save_contact(contact);
    }

    bool is_client = contact.id != -1;
    int id = is_client ? contact.id : contact.chat;

    emit load_messages(is_client, id);

    msg_fld_->show();

    qDebug() << "Index row: " << row << '\n';
}

