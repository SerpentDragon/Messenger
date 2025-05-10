#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatparamswindow.h"
#include "contactentry.h"
#include "messageentryfield.h"
#include "slidepanel.h"
#include "../common_defs.h"
#include "../../../API/common_structure.h"
#include "../../../API/protocols.h"

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

    void set_contacts(const std::vector<Contact>& contacts);

private:

    void setup_panel();

    void setup_icons();

    void resizeEvent(QResizeEvent* event);

    void clear_contacts();

    void display_contacts(const std::vector<Contact>&);

public slots:

    void send_msg(const QString& text);

    void receive_msg(const Message& msg);

    void list_of_contacts(const QString& name, const std::vector<Contact>& contacts_from_srv);

private slots:

    void on_new_chat_button_pressed();

    void on_chat_settings_button_pressed();

    void on_dialog_search_edit_returnPressed();

signals:

    void send_message(const Message& msg);

    void find_contact(const QString& name);

    void send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data);

private:
    Ui::MainWindow* ui;

    SlidePanel* panel_;

    MessageEntryField* msg_fld_;

    ChatParamsWindow* chat_params_wnd_;

    std::vector<Contact> contacts_;
};
#endif // MAINWINDOW_H
