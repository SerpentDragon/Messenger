#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "../common_defs.h"
#include "../../../API/common_structure.h"
#include "../../../API/protocols.h"
#include "timer.h"

#include <deque>
#include <format>
#include <memory>
#include <pqxx/pqxx>
#include <QObject>

using ULL = unsigned long long;

class DBManager : public QObject
{
    Q_OBJECT

public:

    DBManager() = default;

    ~DBManager();

    std::vector<Contact> get_contacts_list();

    void save_RSA_keys(const std::pair<std::string, std::string>& keys);

    std::pair<std::string, std::string> load_RSA_keys();

private:

    void connect(const std::string& address);

    void build_contacts_cash();

public slots:

    void db_connect(bool log_in, int id, const std::string& nickname);

    void save_msg(const SocketMessage& msg);

    void save_contact(const Contact& contact);

    void load_messages(bool is_client, int id);

    void save_chat(int id, const std::string& name, qint64 time, const std::vector<int>& members);

    void delete_chat(int chat_id);

    void delete_messages(const std::vector<int>& msgs_ids);

    void process_system_signal(int signal, const std::vector<int>& ids);

signals:

    void loaded_messages(const std::deque<ClientMessage>& msgs);

    void receive_msg(const ClientMessage& msg, const std::vector<int> ids);

    void display_sent_msg(const ClientMessage& msg, const std::vector<int> ids);

    void save_public_key(int id, const std::string& public_key);

    void update_keys_cash(std::unordered_map<int, std::string> keys_cash);

    void add_new_contact(const Contact& contact);

    void delete_chat_sig();

private:

    std::unique_ptr<pqxx::connection> connection_;

    int id_;
    std::string nickname_;

private:

    struct ContactCash
    {
        std::string name;
        std::string public_key;
        std::string picture;
    };

    std::unordered_map<int, ContactCash> contacts_cash_;
};

#endif // DBMANAGER_H
