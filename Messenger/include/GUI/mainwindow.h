#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatparamswindow.h"
#include "contactentry.h"
#include "messageentryfield.h"
#include "messageform.h"
#include "slidepanel.h"
#include "../common_defs.h"
#include "../../../API/common_structure.h"
#include "../../../API/protocols.h"

#include <deque>
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

    void clear_messages();

    void display_contacts();

    void add_message(const ClientMessage& msg);

    void display_messages();

    Contact* get_contact_from_lst();

public slots:

    void send_msg(const QString& text);

    void receive_msg(const ClientMessage& msg);

    void list_of_contacts(const std::string& name, const std::vector<Contact>& contacts_from_srv);

    void loaded_messages(const std::deque<ClientMessage>& msgs);

    void display_sent_msg(const ClientMessage& msg);

private slots:

    void on_new_chat_button_pressed();

    void on_chat_settings_button_pressed();

    void on_dialog_search_edit_returnPressed();

    void on_dialogs_list_clicked(const QModelIndex& index);

signals:

    void send_message(SocketMessage& msg);

    void set_contacts_from_db();

    void find_contact(const QString& name);

    void save_contact(Contact& contact);

    void send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data);

    void load_messages(bool is_client, int id);

private:

    Ui::MainWindow* ui;

    SlidePanel* panel_;

    MessageEntryField* msg_fld_;

    ChatParamsWindow* chat_params_wnd_;

    std::vector<Contact> contacts_;

    std::deque<ClientMessage> messages_;
    std::deque<std::shared_ptr<MessageForm>> msg_forms_;
};

#endif // MAINWINDOW_H
