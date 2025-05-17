#include "../../include/Client/dbmanager.h"

#include <QDebug>

DBManager::~DBManager()
{
    connection_->close();
}

std::vector<Contact> DBManager::get_contacts_list()
{
    if (!connection_->is_open()) return {};

    std::vector<Contact> contacts;
    pqxx::work txn(*connection_);

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

    auto result = txn.exec("SELECT * FROM Chat;");
    for (const auto& row : result)
    {
        int chat_id = row["chat_id"].as<int>();
        auto part = txn.exec("SELECT contact_id FROM ChatParticipants WHERE chat_id = "
                             + txn.quote(chat_id) + " AND personal_id = " + txn.quote(id_) + ")");

        std::vector<int> participants(part.size());
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

    return contacts;
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

    pqxx::work txn(*connection_);

    auto result = txn.exec(std::string("SELECT contact_id, nickname, picture ") +
             "FROM Contact WHERE personal_id = " + txn.quote(id_) + ";");

    std::string qr = std::string("SELECT contact_id, nickname, picture ") +
                  "FROM Contact WHERE personal_id = " + txn.quote(id_) + ";";

    qDebug() << "QR: " << qr << '\n';

    for (const auto& row : result)
    {
        contacts_cash_.insert({ row["contact_id"].as<int>(),
                               { row["nickname"].as<std::string>(), row["picture"].as<std::string>() } });
    }

    for(const auto& c : contacts_cash_)
    {
        qDebug() << c.first << ' ' << c.second.name << '\n';
    }

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
                 txn.quote("") + ");"
                 );

        txn.commit();
    }

    qDebug() << "INSERTED\n";
}

void DBManager::save_msg(const SocketMessage& msg)
{
    if (!connection_->is_open()) return;
    pqxx::work txn(*connection_);
    pqxx::result result;

    for(int recv : msg.receiver)
    {
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
            txn.commit();
        } catch (const std::exception& ex)
        {
            qDebug() << ex.what() << '\n';
            return;
        }
    }

    int new_id = result[0][0].as<int>();

    if (msg.receiver[0] == id_)
    {
        ClientMessage cl_msg = ClientMessage(new_id, contacts_cash_[msg.sender].name, msg.sender,
                                             nickname_, id_, msg.text, msg.timestamp, msg.chat);

        emit receive_msg(cl_msg);
    }
    else if (msg.sender == id_)
    {
        ClientMessage cl_msg = ClientMessage(new_id, nickname_, id_,
                                             contacts_cash_[msg.sender].name, id_, msg.text, msg.timestamp, msg.chat);

        emit display_sent_msg(cl_msg);
    }
}

void DBManager::save_contact(Contact& contact)
{
    pqxx::work txn(*connection_);

    try
    {
        txn.exec("INSERT INTO Contact VALUES(" +
                 txn.quote(contact.id) + ", " +
                 txn.quote(contact.name) + ", " +
                 txn.quote("") + ", " + // picutre
                 txn.quote(id_) + ");");
        txn.commit();

        contact.saved_in_db = true;

        contacts_cash_.insert({ contact.id, { contact.name, contact.picture } });
    }
    catch(const std::exception& ex)
    {
        qDebug() << ex.what() << '\n';
        return;
    }
}

void DBManager::load_messages(bool is_client, int id)
{
    pqxx::work txn(*connection_);

    std::deque<ClientMessage> msgs;

    qDebug() << "LOAD MSGS" << is_client << ' ' << id << '\n';

    // should be improved to retrieve not all the messages
    // but just some of them to display on the screen

    auto result = txn.exec("SELECT nickname FROM Contact WHERE contact_id = " +
                           txn.quote(id) + " AND personal_id = " + txn.quote(id_) + ";");

    qDebug() << result.empty() << '\n';

    if (!result.empty())
    {
        std::string contact_name = result[0]["nickname"].as<std::string>();

        qDebug() << "NAME: " << contact_name << '\n';

        try
        {
            result = txn.exec(
                std::string("SELECT id, receiver, sender, text, timestamp, chat_id FROM Message WHERE ") +
                "(receiver = " + txn.quote(id) + " OR sender = " + txn.quote(id) + ") " +
                "AND personal_id = " + txn.quote(id_) +
                " AND chat_id IS " + (is_client ? "" : "NOT") + " NULL ORDER BY timestamp LIMIT " +
                txn.quote(max_msg_count) + ";");
        }
        catch(const std::exception& ex)
        {
            qDebug() << ex.what() << '\n';
            return;
        }

        qDebug() << result.size() << '\n';

        for (const auto& row : result)
        {
            msgs.push_back(
                ClientMessage
                {
                    row["id"].as<int>(),
                    row["sender"].as<int>() == id_ ? nickname_ : contact_name,
                    row["sender"].as<int>(),
                    row["receiver"].as<int>() == id_ ? nickname_ : contact_name,
                    row["receiver"].as<int>(),
                    row["text"].as<std::string>(),
                    row["timestamp"].as<ULL>(),
                    row["chat_id"].is_null() ? -1 : row["chat_id"].as<int>()
                }
            );
        }
    }

    emit loaded_messages(msgs);
}
