#include "../../include/GUI/mainwindow.h"
#include "ui/ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setup_panel();
    setup_icons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setup_panel()
{
    panel = new SlidePanel(ui->menubar->height() + ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height(), this);
    panel->show();

    connect(ui->menu_button, &QPushButton::clicked, [this]() {
        if (panel->pos().x() < 0)
        {
            panel->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
            panel->show_panel();
        }
        else
            panel->hide_panel();
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
    panel->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
}
