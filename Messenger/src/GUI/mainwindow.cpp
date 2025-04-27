#include "../../include/GUI/mainwindow.h"
#include "ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_panel();
    setup_icons();

    msg_edit_ = new AutoExpandTextEdit(ui->dialog_window);
    msg_edit_->show();
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
    ui->chat_settings_button->setIcon(QIcon(":/main_window/main_window/chat_settings.png"));
    ui->P2P_button->setIcon(QIcon(":/main_window/main_window/P2P.png"));
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    msg_edit_->set_coordinates(ui->dialog_window->height(), ui->dialog_window->width());
    panel_->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
}
