#include "../../include/GUI/mainwindow.h"
#include "ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_panel();
    setup_icons();

    msg_fld_ = new MessageEntryField(ui->dialog_window);
    msg_fld_->show();

    connect(msg_fld_, &MessageEntryField::send_msg,
            this, &MainWindow::send_msg);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_contacts(const std::vector<Contact> &contacts)
{
    QStandardItemModel* model = new QStandardItemModel(this);
    ui->dialogs_list->setModel(model);

    for(const auto& contact : contacts)
    {
        QStandardItem* item = new QStandardItem;
        item->setSizeHint({ ui->dialogs_list->size().width(), 50 });
        model->appendRow(item);

        QModelIndex index = model->indexFromItem(item);

        QPixmap photo(contact.picture.c_str());
        ContactEntry* ce = new ContactEntry(photo, QString::fromStdString(contact.name),
                                            contact.id, contact.chat);

        ui->dialogs_list->setIndexWidget(index, ce);
    }
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

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    msg_fld_->calculate_widgets_pos();
    panel_->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
}

void MainWindow::send_msg(const QString& text)
{
    Message msg
    {
        // .sender =
        // .receiver =
        .text = text.toStdString(),
        .timestamp = generate_timestamp(),
        // .chat =
    };

    // here we should get the list of receivers

    emit send_message(msg);
}

void MainWindow::receive_msg(const Message& msg)
{
    // here we should display in the corresponding chat

    // qDebug() << sender << ' ' << msg << '\n';
}

void MainWindow::on_new_chat_button_pressed()
{
    chat_params_wnd_ = new ChatParamsWindow(this);
    chat_params_wnd_->show();
}

void MainWindow::on_chat_settings_button_pressed()
{
    chat_params_wnd_ = new ChatParamsWindow(this);
    chat_params_wnd_->show();
}

void MainWindow::on_dialog_search_edit_textChanged(const QString& text)
{
    // emit find_contact(text);
}

