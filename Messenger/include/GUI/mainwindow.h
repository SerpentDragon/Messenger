#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chatparamswindow.h"
#include "contactentry.h"
#include "messageentryfield.h"
#include "messageform.h"
#include "slidepanel.h"
#include "../common_defs.h"
#include "../Client/timer.h"
#include "../../../API/common_structure.h"
#include "../../../API/protocols.h"

#include <deque>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QListView>
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

    void set_id(const int id);

    void update_message_field();

protected:

    bool eventFilter(QObject *watched, QEvent *event) override;

private:

    void setup_panel();

    void setup_icons();

    void setup_timer(const ClientMessage& msg, const std::vector<int>& ids);

    void resizeEvent(QResizeEvent* event) override;

    void clear_list_view(QListView* list);

    void display_contacts();

    void add_message(const ClientMessage& msg, const std::vector<int> ids);

    void display_messages();

    Contact* get_contact_from_lst();

public slots:

    void send_msg(bool vanishing, const QString& text);

    void receive_msg(const ClientMessage& msg, const std::vector<int> ids);

    void list_of_contacts(const std::string& name, const std::vector<Contact>& contacts_from_srv);

    void loaded_messages(const std::deque<ClientMessage>& msgs);

    void display_sent_msg(const ClientMessage& msg, const std::vector<int>& ids);

    void add_contact(const Contact& contact);

    void create_new_chat(const QString& name, qint64 time, const std::vector<int>& members);

    void delete_messages(int type, const std::vector<int>& msgs_ids);

    void set_P2P_status(int id, P2P_CONNECTION_TYPE cn_tp, P2P_CONNECTION_STATUS cn_st);

private slots:

    void on_new_chat_button_pressed();

    void on_chat_settings_button_pressed();

    void on_P2P_button_pressed();

    void on_dialog_search_edit_returnPressed();

    void on_dialogs_list_clicked(const QModelIndex& index);

signals:

    void send_message(SocketMessage& msg);

    void set_contacts_from_db();

    void find_contact(const QString& name);

    void save_contact(const Contact& contact);

    void send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data);

    void load_messages(bool is_client, int id);

    void new_chat(const QString& name, qint64 time, const std::vector<int> members);

    void delete_messages_sig(const std::vector<int>& ids);

    void establish_p2p_connection(int id);

    void close_p2p_connection(int id);

private:

    Ui::MainWindow* ui;

    int my_id_;

    SlidePanel* panel_;

    MessageEntryField* msg_fld_;

    ChatParamsWindow* chat_params_wnd_;

    std::vector<Contact> contacts_;

    std::deque<ClientMessage> messages_;
    std::deque<std::shared_ptr<MessageForm>> msg_forms_;
};

#endif // MAINWINDOW_H
