#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "../common_defs.h"
#include "../../../API/common_structure.h"

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

    void save_msg(bool display, const SocketMessage& msg);

    void save_contact(Contact& contact);

    void load_messages(bool is_client, int id);

signals:

    void loaded_messages(const std::deque<ClientMessage>& msgs);

    void receive_msg(const ClientMessage& msg);

    void display_sent_msg(const ClientMessage& msg);

    void save_public_key(int id, const std::string& public_key);

    void update_keys_cash(std::unordered_map<int, std::string> keys_cash);

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
