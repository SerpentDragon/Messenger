#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatparamswindow.h"
#include "messageentryfield.h"
#include "slidepanel.h"
#include "../common_defs.h"

#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QMainWindow>
#include <QPainter>
#include <QStandardItem>
#include <QWidget>

using ULL = unsigned long long;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget* parent = nullptr);

    ~MainWindow();

private:

    void setup_panel();

    void setup_icons();

    void resizeEvent(QResizeEvent* event);

public slots:

    void send_msg_text(const QString& text);

    void receive_msg(const Message& msg);

private slots:

    void on_new_chat_button_pressed();

    void on_chat_settings_button_pressed();

signals:

    void send_message(const Message& msg);

private:
    Ui::MainWindow* ui;

    SlidePanel* panel_;

    MessageEntryField* msg_fld_;

    ChatParamsWindow* chat_params_wnd_;
};
#endif // MAINWINDOW_H
