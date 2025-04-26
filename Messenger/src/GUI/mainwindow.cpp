#include "../../include/GUI/mainwindow.h"
#include "ui/ui_mainwindow.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStandardItem>
#include <QWidget>

class CustomLabel : public QWidget {

public:
    explicit CustomLabel(const QString& text, QWidget *parent = nullptr)
        : QWidget(parent) {
        m_text = text;
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);

        QFont font = painter.font();
        font.setPointSize(12);
        painter.setFont(font);

        painter.setPen(Qt::black);

        int textHeight = painter.fontMetrics().height();
        int verticalCenter = (height() - textHeight) / 2;

        painter.drawText(0, verticalCenter + textHeight, m_text);
    }

private:
    QString m_text;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStandardItemModel* itemModel_ = new QStandardItemModel(this);
    ui->dialogs_list->setModel(itemModel_);

    std::vector<QString> titles = {
        "Alice", "Mark", "Tom", "Leonard", "Michael", "Olivia", "Emma",
        "Sophia", "Luna", "Mia", "John", "Harry Potter", "Voldemort", "Charlotte"
    };

    std::vector<CustomLabel*> labels;

    for(std::size_t i = 0; i < titles.size(); i++)
    {
        QStandardItem* item = new QStandardItem();
        item->setSizeHint({ ui->dialogs_list->size().width(), 50 });
        itemModel_->appendRow(item);

        CustomLabel* cl = new CustomLabel(titles[i]);
        labels.emplace_back(cl);

        ui->dialogs_list->setIndexWidget(item->index(), labels[i]);
    }



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
    ui->menu_button->setIcon(QIcon(":/main_window/resources/main_window/menu.png"));
    ui->new_chat_button->setIcon(QIcon(":/main_window/resources/main_window/new_chat.png"));
    ui->chat_settings_button->setIcon(QIcon(":/main_window/resources/main_window/chat_settings.png"));
    ui->P2P_button->setIcon(QIcon(":/main_window/resources/main_window/P2P.png"));
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    panel->set_coordinates(ui->dialogs_list->y(), ui->dialogs_list->width(), ui->dialogs_list->height());
}
