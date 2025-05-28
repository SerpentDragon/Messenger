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

    ui->new_chat_button->show();
    ui->chat_settings_button->hide();
    ui->P2P_button->hide();

    connect(msg_fld_, &MessageEntryField::send_msg,
            this, &MainWindow::send_msg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_contacts(const std::vector<Contact> &contacts)
{
    contacts_ = contacts;

    display_contacts();
}

void MainWindow::set_id(const int id)
{
    my_id_ = id;
}

void MainWindow::update_message_field()
{
    messages_.clear();
    msg_forms_.clear();
    display_messages();

    ui->new_chat_button->show();
    ui->chat_settings_button->hide();
    ui->P2P_button->hide();

    ui->dialog_name_label->setText("");
    msg_fld_->hide();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->dialogs_list && event->type() == QEvent::FocusOut)
    {
        ui->dialogs_list->clearSelection();
        ui->dialogs_list->clearFocus();

        update_message_field();
    }

    return QWidget::eventFilter(watched, event);
}

void MainWindow::setup_panel()
{
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
}

void MainWindow::setup_icons()
{
    ui->menu_button->setIcon(QIcon(":/main_window/main_window/menu.png"));
    ui->new_chat_button->setIcon(QIcon(":/main_window/main_window/new_chat.png"));
    ui->chat_settings_button->setIcon(QIcon(":/main_window/main_window/params.png"));
    ui->P2P_button->setIcon(QIcon(":/main_window/main_window/P2P.png"));
}

void MainWindow::setup_timer(const ClientMessage &msg, const std::vector<int>& ids)
{
    Timer* timer = new Timer(this);
    qint64 time = QDateTime::currentSecsSinceEpoch() +
                  msg.text.size() / avg_reading_speed + addl_time;
    timer->start(time, SYSTEM_MSG::DELETE_MESSAGE, ids);
    QObject::connect(timer, &Timer::timeout, this, &MainWindow::delete_messages);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
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
}

void MainWindow::clear_list_view(QListView* lists)
{
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
}

void MainWindow::display_contacts()
{
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
    }
}

void MainWindow::add_message(const ClientMessage& msg, const std::vector<int> ids)
{
    Contact* const contact = get_contact_from_lst();
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

    if (msg.vanishing)
    {
        setup_timer(msg, ids);
    }
}

void MainWindow::display_messages()
{
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

        if (messages_[i].vanishing)
        {
            setup_timer(messages_[i], { messages_[i].id });
        }
    }

    ui->messages_window->scrollToBottom();
}

Contact* MainWindow::get_contact_from_lst()
{
    QModelIndex index = ui->dialogs_list->currentIndex();
    int row = index.row();
    return row < 0 ? nullptr : &contacts_[row];
}

void MainWindow::send_msg(bool vanishing, const QString& text)
{
    Contact* const contact = get_contact_from_lst();
    if (contact == nullptr) return;

    int client_id = contact->id;
    int chat_id = contact->chat;

    SocketMessage msg
    {
        .receiver = chat_id == -1 ? std::vector{ client_id } : contact->participants,
        .text = text.toStdString(),
        .timestamp = generate_timestamp(),
        .chat = chat_id,
        .vanishing = vanishing,
        .p2p = contact->p2p
    };

    emit send_message(msg);
}

void MainWindow::receive_msg(const ClientMessage& msg, const std::vector<int>& ids)
{
    if (Contact* const contact = get_contact_from_lst(); contact != nullptr)
    {
        if (!msg_fld_->isHidden() && (msg.sender_id == contact->id) || msg.chat == contact->chat)
        {
            add_message(msg, ids);
        }
    }
}

void MainWindow::list_of_contacts(const std::string& name, const std::vector<Contact>& contacts_from_srv)
{
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
}

void MainWindow::loaded_messages(const std::deque<ClientMessage>& msgs)
{
    Contact* const contact = get_contact_from_lst();
    if (contact == nullptr) return;

    ui->dialog_name_label->setText(QString::fromStdString(contact->name));

    messages_ = msgs;
    msg_forms_.clear();

    for(int i = 0; i < max_msg_count && i < messages_.size(); i++)
    {
        msg_forms_.emplace_back(new MessageForm(&messages_[i], messages_[i].sender_id != my_id_));
    }

    display_messages();
}

void MainWindow::display_sent_msg(const ClientMessage &msg, const std::vector<int>& ids)
{
    add_message(msg, ids);
}

void MainWindow::add_contact(const Contact& contact)
{
    if (std::any_of(contacts_.begin(), contacts_.end(),
                    [contact](const Contact& cn){ return cn.id == contact.id; }))
    {
        return;
    }

    contacts_.push_back(contact);
    contacts_.back().saved_in_db = true;

    emit save_contact(contacts_.back());

    display_contacts();
}

void MainWindow::create_new_chat(const QString &name, qint64 time, const std::vector<int>& members)
{
    std::vector<int> mems;
    std::copy_if(members.begin(), members.end(), std::back_inserter(mems), [](int mem){ return mem > 0; });
}

void MainWindow::delete_messages(int type, const std::vector<int> &msgs_ids)
{
    if (type == SYSTEM_MSG::DELETE_MESSAGE)
    {
        int sender = messages_[0].sender_id;
        int receiver = messages_[0].receiver_id;
        int chat = messages_[0].chat;

        emit delete_messages_sig(msgs_ids);
        emit load_messages(chat == -1, chat != -1 ? chat : (sender == my_id_ ? receiver : sender));
    }
}

void MainWindow::set_P2P_status(int id, P2P_CONNECTION_TYPE cn_tp, P2P_CONNECTION_STATUS cn_st)
{
    QString name;

    for(auto& cn : contacts_)
    {
        if (cn.id == id)
        {
            cn.p2p = cn_tp;
            name = QString::fromStdString(cn.name);
            break;
        }
    }

    if (cn_st == P2P_CONNECTION_STATUS::SUCCESSFUL)
    {
        QMessageBox::information(this, "Success", "P2P connection to " + name + " is successful!");
    }
    else if (cn_st == P2P_CONNECTION_STATUS::SERVER_FALLBACK)
    {
        QMessageBox::critical(this, "Error", "Unable to establish P2P connection to " + name +
                              ". Your messages will be transmitted through the server");
    }
    else if (cn_st == P2P_CONNECTION_STATUS::DISCONNECTED)
    {
        QMessageBox::critical(this, "Error", "Disconnected from " + name);
    }
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

void MainWindow::on_P2P_button_pressed()
{
    if (Contact* contact = get_contact_from_lst(); contact)
    {
        if (contact->p2p == P2P_CONNECTION_TYPE::FALSE)
        {
            contact->p2p = P2P_CONNECTION_TYPE::TRUE;
            emit establish_p2p_connection(contact->id);
        }
        else
        {
            contact->p2p = P2P_CONNECTION_TYPE::FALSE;
            emit close_p2p_connection(contact->id);
        }
    }
}

void MainWindow::on_dialog_search_edit_returnPressed()
{
    auto text = ui->dialog_search_edit->text();

    if (text.size() == 0)
    {
        emit set_contacts_from_db();
    }
    else emit send_system_msg(SYSTEM_MSG::FIND_CONTACT, { text });
}

void MainWindow::on_dialogs_list_clicked(const QModelIndex& index)
{
    int row = index.row();

    Contact contact = contacts_[row];

    if (!contact.saved_in_db)
    {
        contact.saved_in_db = true;
        emit save_contact(contact);
    }

    bool is_client = contact.id != -1;
    int id = is_client ? contact.id : contact.chat;

    emit load_messages(is_client, id);

    ui->new_chat_button->hide();
    if (contact.id != -1)
    {
        ui->chat_settings_button->hide();
        ui->P2P_button->show();
    }
    else
    {
        ui->chat_settings_button->show();
        ui->P2P_button->hide();
    }

    msg_fld_->show();
}
