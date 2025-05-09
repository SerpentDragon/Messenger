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

    auto result = txn.exec("SELECT * FROM Contact;");

    qDebug() << "RETRIEVED!\n";

    for (const auto& row : result)
    {
        Contact contact
        {
            .id = row["contact_id"].as<int>(),
            .name = row["nickname"].as<std::string>(),
            .picture = row["picture"].as<std::string>(),
            .chat = -1
        };

        contacts.emplace_back(contact);
    }

    qDebug() << "Contacts are ready!\n";

    result = txn.exec("SELECT * FROM Chat;");
    for (const auto& row : result)
    {
        Contact contact
        {
            .id = -1,
            .name = row["name"].as<std::string>(),
            .picture = row["picture"].as<std::string>(),
            .chat = row["chat_id"].as<int>()
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

void DBManager::db_connect(bool log_in, int id, const std::string& nickname)
{
    std::string address = "dbname=messenger_client user=dmitry password=1223684DS hostaddr=127.0.0.1 port=5432";

    // std::string address = std::format("dbname=messenger_client user={} password={} hostaddr=127.0.0.1 port=5432",
                                      // nickname, password);

    connect(address);

    if (log_in == false)
    {
        id_ = id;

        pqxx::work txn(*connection_);
        txn.exec("INSERT INTO Personal VALUES (" +
                 txn.quote(id) + ", " +
                 txn.quote(nickname) + ", " +
                 txn.quote("") + ");"
                 );

        txn.commit();
    }

}

void DBManager::save_msg(const Message& msg)
{
    if (!connection_->is_open()) return;
    pqxx::work txn(*connection_);

    txn.exec("INSERT INTO Message VALUES(" +
             txn.quote(msg.sender) + ", " +
             txn.quote(msg.receiver) + ", " +
             txn.quote(msg.text) + ", " +
             txn.quote(msg.timestamp) + ", " +
             txn.quote(msg.chat) + ");"
             );
    txn.commit();
}
