#include "../../include/Client/dbmanager.h"

#include <QDebug>

DBManager::~DBManager()
{
    connection_->close();
}

std::vector<Contact> DBManager::get_contacts_list()
{
    qDebug() << __func__ << '\n';

    if (!connection_->is_open()) return {};

    std::vector<Contact> contacts;

    pqxx::work txn(*connection_);

    qDebug() << "CONTACTS CASH 1\n";
    for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
    qDebug() << "-------------\n";

    for(const auto& contact : contacts_cash_)
    {
        Contact cn
        {
            .id = contact.first,
            .name = contact.second.name,
            .picture = contact.second.picture,
            .chat = -1,
            .participants = {},
            .saved_in_db = true
        };

        contacts.emplace_back(cn);
    };

    try
    {
        auto result = txn.exec("SELECT * FROM Chat;");
        for (const auto& row : result)
        {
            int chat_id = row["chat_id"].as<int>();
            auto part = txn.exec("SELECT contact_id FROM ChatParticipants WHERE chat_id = "
                                 + txn.quote(chat_id) + ";");

            std::vector<int> participants;
            for(const auto& p : part) participants.push_back(p["contact_id"].as<int>());

            Contact contact
                {
                    .id = -1,
                    .name = row["name"].as<std::string>(),
                    .picture = row["picture"].as<std::string>(),
                    .chat = chat_id,
                    .participants = participants,
                    .saved_in_db = true
                };

            contacts.emplace_back(contact);
        }

    }
    catch(const std::exception& ex)
    {
        qDebug() << __func__ << ex.what() << '\n';
        return {};
    }

    qDebug() << "leave" << __func__ << '\n';

    return contacts;
}

void DBManager::save_RSA_keys(const std::pair<std::string, std::string> &keys)
{
    pqxx::work txn(*connection_);

    txn.exec("UPDATE Personal SET private_key = " +
             txn.quote(keys.first) + ", " +
             "public_key = " + txn.quote(keys.second) +
             " WHERE id = " + txn.quote(id_) + ";");
    txn.commit();
}

std::pair<std::string, std::string> DBManager::load_RSA_keys()
{
    pqxx::work txn(*connection_);

    auto result = txn.exec("SELECT private_key, public_key FROM Personal WHERE id = " +
             txn.quote(id_) + ";");

    std::string private_key = result[0]["private_key"].as<std::string>();
    std::string public_key = result[0]["public_key"].as<std::string>();

    return { private_key, public_key };
}

void DBManager::connect(const std::string& address)
{
    connection_ = std::make_unique<pqxx::connection>(address);

    if(!connection_->is_open())
    {
        throw std::runtime_error("Error connecting to " + address + "!\n");
    }
}

void DBManager::build_contacts_cash()
{
    qDebug() << __func__ << '\n';

    std::unordered_map<int, std::string> keys_cash;

    pqxx::work txn(*connection_);

    auto result = txn.exec(std::string("SELECT contact_id, nickname, public_key, picture ") +
             "FROM Contact WHERE personal_id = " + txn.quote(id_) + ";");

    std::string qr = std::string("SELECT contact_id, nickname, public_key, picture ") +
                  "FROM Contact WHERE personal_id = " + txn.quote(id_) + ";";

    qDebug() << "QR: " << qr << '\n';

    qDebug() << "CONTACTS CASH 2\n";
    for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
    qDebug() << "-------------\n";

    for (const auto& row : result)
    {

        contacts_cash_.insert({ row["contact_id"].as<int>(),
                               {
                                row["nickname"].as<std::string>(),
                                row["public_key"].as<std::string>(),
                                row["picture"].as<std::string>()
                               }
                              });

        qDebug() << "CONTACTS CASH 3\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";

        keys_cash.insert({ row["contact_id"].as<int>(), row["public_key"].as<std::string>() });
    }

    qDebug() << "CONTACTS CASH 4\n";
    for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
    qDebug() << "-------------\n";

    emit update_keys_cash(keys_cash);

    qDebug() << "leave " << __func__ << '\n';
}

void DBManager::db_connect(bool log_in, int id, const std::string& nickname)
{
    std::string address = "dbname=messenger_client user=dmitry password=1223684DS hostaddr=127.0.0.1 port=5432";

    // std::string address = std::format("dbname=messenger_client user={} password={} hostaddr=127.0.0.1 port=5432",
                                      // nickname, password);

    connect(address);

    id_ = id;
    nickname_ = nickname;

    build_contacts_cash();

    qDebug() << "INSERT INTO PERSONAL\n";

    if (log_in == false)
    {
        pqxx::work txn(*connection_);
        txn.exec("INSERT INTO Personal VALUES (" +
                 txn.quote(id) + ", " +
                 txn.quote(nickname) + ", " +
                 txn.quote("private") + ", " +
                 txn.quote("public") + ", " +
                 txn.quote("") + ");"
                 );

        txn.commit();
    }

    qDebug() << "INSERTED\n";
}

void DBManager::save_msg(const SocketMessage& msg)
{
    qDebug() << __func__ << '\n';
    if (!connection_->is_open()) return;

    pqxx::work txn(*connection_);
    pqxx::result result;
    int receiver;

    qDebug() << "RECV SIZE:" << msg.receiver.size();
    for(int recv : msg.receiver) qDebug() << "RECV: " << recv;

    for(int recv : msg.receiver)
    {
        if (recv == id_ && msg.sender == id_) continue;

        std::string qr = "INSERT INTO Message (receiver, sender, text, timestamp, chat_id, personal_id) VALUES(" +
                         txn.quote(recv) + ", " +
                         txn.quote(msg.sender) + ", " +
                         txn.quote(msg.text) + ", " +
                         txn.quote(msg.timestamp) + ", " +
                         (msg.chat == -1 ? "NULL" : txn.quote(msg.chat)) + ", " +
                         txn.quote(id_) + ") RETURNING id;";

        qDebug() << qr << '\n';

        try
        {
            result = txn.exec(qr);
            receiver = recv;
        } catch (const std::exception& ex)
        {
            qDebug() << ex.what() << '\n';
            return;
        }
    }

    txn.commit();

    int new_id = result[0][0].as<int>();

    if (msg.sender != id_)
    {
        qDebug() << "CONTACTS CASH 5\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";
        ClientMessage cl_msg = ClientMessage(new_id, contacts_cash_[msg.sender].name, msg.sender,
                                             nickname_, id_, msg.text, msg.timestamp, msg.chat);
        qDebug() << "CONTACTS CASH 6\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";

        emit receive_msg(cl_msg);
    }
    else
    {
        qDebug() << "CONTACTS CASH 7\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";
        ClientMessage cl_msg = ClientMessage(new_id, nickname_, id_,
                                             contacts_cash_[receiver].name, receiver, msg.text,
                                             msg.timestamp, msg.chat);
        qDebug() << "CONTACTS CASH 7\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";
        emit display_sent_msg(cl_msg);
    }

    qDebug() << "leave" << __func__ << '\n';
}

void DBManager::save_contact(const Contact& contact)
{
    qDebug() << __func__ << '\n';

    pqxx::work txn(*connection_);

    if (contact.id == -1 || contact.id == id_) return;

    auto result = txn.exec("SELECT contact_id FROM Contact WHERE contact_id = " +
                           txn.quote(contact.id) + "AND personal_id = " +
                           txn.quote(id_) + ";");
    if (!result.empty())
    {
        qDebug() << "Contact" << contact.id << "EXISTS";
        return;
    }

    qDebug() << "Contact" << contact.id << "TO BE SAVED";

    try
    {
        txn.exec("INSERT INTO Contact VALUES(" +
                 txn.quote(contact.id) + ", " +
                 txn.quote(contact.name) + ", " +
                 txn.quote(contact.public_key) + ", " +
                 txn.quote("") + ", " + // picutre
                 txn.quote(id_) + ");");
        txn.commit();

        qDebug() << "CONTACTS CASH 8\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";

        contacts_cash_[contact.id] = { contact.name, contact.public_key, contact.picture };

        qDebug() << "CONTACTS CASH 8\n";
        for(auto v : contacts_cash_) qDebug() << v.first << v.second.name;
        qDebug() << "-------------\n";

        emit save_public_key(contact.id, contact.public_key);
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what() << '\n';
        return;
    }

    qDebug() << "leave" << __func__ << '\n';
}

void DBManager::load_messages(bool is_client, int id)
{
    qDebug() << __func__ << '\n';

    pqxx::work txn(*connection_);

    std::deque<ClientMessage> msgs;
    pqxx::result result;

    qDebug() << "LOAD MSGS" << is_client << ' ' << id << '\n';

    // should be improved to retrieve not all the messages
    // but just some of them to display on the screen

    if (is_client)
    {
        //result = txn.exec("SELECT nickname FROM Contact WHERE contact_id = " +
                               //txn.quote(id) + " AND personal_id = " + txn.quote(id_) + ";");

        //if (!result.empty())
        //{
        // std::string contact_name = result[0]["nickname"].as<std::string>();
        std::string contact_name = contacts_cash_[id].name;

        result = txn.exec(
            std::string("SELECT id, receiver, sender, text, timestamp, chat_id FROM Message WHERE ") +
            "(receiver = " + txn.quote(id) + " OR sender = " + txn.quote(id) + ") " +
            "AND personal_id = " + txn.quote(id_) +
            " AND chat_id IS " + (is_client ? "" : "NOT") + " NULL ORDER BY timestamp " +
            // "LIMIT " + txn.quote(max_msg_count) +
            ";");

        for (const auto& row : result)
        {
            int sender = row["sender"].as<int>();
            int receiver = row["receiver"].as<int>();

            msgs.push_back(
                ClientMessage
                {
                    row["id"].as<int>(),
                    sender == id_ ? nickname_ : contact_name,
                    sender,
                    receiver == id_ ? nickname_ : contact_name,
                    receiver,
                    row["text"].as<std::string>(),
                    row["timestamp"].as<ULL>(),
                    row["chat_id"].is_null() ? -1 : row["chat_id"].as<int>()
                });
        }
        //}
    }
    else
    {
        result = txn.exec(std::string("SELECT * FROM ") +
                "(SELECT DISTINCT ON (text, sender) * FROM Message ORDER BY text, sender)" +
                "WHERE chat_id = " + txn.quote(id) +
                "ORDER BY timestamp;");

        for (const auto& row : result)
        {
            int sender = row["sender"].as<int>();
            int receiver = row["receiver"].as<int>();

            msgs.push_back(
                ClientMessage
                {
                    row["id"].as<int>(),
                    sender == id_ ? nickname_ : contacts_cash_[sender].name,
                    sender,
                    receiver == id_ ? nickname_ : contacts_cash_[receiver].name,
                    receiver,
                    row["text"].as<std::string>(),
                    row["timestamp"].as<ULL>(),
                    row["chat_id"].is_null() ? -1 : row["chat_id"].as<int>()
                });
        }
    }

    emit loaded_messages(msgs);

    qDebug() << "leave" << __func__ << '\n';
}

void DBManager::save_chat(int chat_id, const std::string& name, const std::vector<int>& members)
{
    qDebug() << __func__ << chat_id << '\n';

    pqxx::work txn(*connection_);

    try
    {
        txn.exec("INSERT INTO Chat (chat_id, name, picture, personal_id) VALUES (" +
                 txn.quote(chat_id) + ", " + txn.quote(name) + ", '', " + txn.quote(id_) + ");");
    }
    catch(const std::exception& ex)
    {
        qDebug() << __func__ << ex.what();
        return;
    }



    qDebug() << __func__ << "MEMBERS SIZE:" << members.size();

    try
    {
        for(int mem : members)
        {
            txn.exec("INSERT INTO ChatParticipants (contact_id, chat_id) VALUES (" +
                     txn.quote(mem) + ", " + txn.quote(chat_id) + ");");
        }
    }
    catch(const std::exception& ex)
    {
        qDebug() << __func__ << ex.what();
    }
    txn.commit();

    for(int m : members) qDebug() << "MEMBER:" << m;

    Contact contact
    {
        .id = -1,
        .name = name,
        .picture = "",
        .chat = chat_id,
        .participants = members,
        .saved_in_db = true
    };

    emit add_new_contact(contact);

    qDebug() << "leave" << __func__ << '\n';
}
