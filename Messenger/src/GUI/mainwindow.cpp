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

    connect(msg_fld_, &MessageEntryField::send_msg_text,
            this, &MainWindow::send_msg_text);
}

MainWindow::~MainWindow()
{
    delete ui;
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

void MainWindow::send_msg_text(const QString &text)
{
    Message msg;

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
