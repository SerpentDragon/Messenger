#include "database.h"

DBServerManager::~DBServerManager()
{
    connection_->close();
}

void DBServerManager::connect(const std::string& address)
{
    connection_ = std::make_unique<pqxx::connection>(address);

    if(!connection_->is_open())
    {
        throw std::runtime_error("Error connecting to " + address + "!\n");
    }
}

void DBServerManager::save_RSA_keys(const std::pair<std::string, std::string>& keys)
{
    pqxx::work txn(*connection_);

    auto result = txn.exec("SELECT * FROM SystemData;");
    if (!result.empty())
    {
        txn.exec("INSERT INTO SystemData (private_key, public_key) VALUES (" +
            txn.quote(keys.first) + ", " + txn.quote(keys.second) + ");");
    }
    else
    {
        txn.exec("UPDATE SystemData set private_key = " + txn.quote(keys.first) +
            ", public_key = " + txn.quote(keys.second) + ";");
    }

    txn.commit();
}

std::string DBServerManager::log_in_client(int& id, const std::string& nickname)
{
    pqxx::work txn(*connection_);
    pqxx::result res = txn.exec("SELECT id, password FROM Client WHERE nickname = " + txn.quote(nickname));

    id = res.empty() ? -1 : res[0][0].as<int>();

    return res.empty() ? "" : res[0][1].as<std::string>();
}

bool DBServerManager::sign_up_client(int& id, const std::string& nickname, const std::string& password)
{
    pqxx::transaction txn(*connection_);

    try
    {
        auto result = txn.exec("INSERT INTO Client (nickname, public_key, password, picture) VALUES (" +
                 txn.quote(nickname) + ", " + 
                 txn.quote("public") + ", " + 
                 txn.quote(password) + ", " + 
                 txn.quote("") + ") RETURNING id;");

        txn.commit();

        id = result[0][0].as<int>();
    }
    catch(const std::exception& ex)
    {
        id = -1;
        return false;
    }

    return true;
}

void DBServerManager::load_RSA_key(int id, const std::string key)
{
    pqxx::work txn(*connection_);

    txn.exec("UPDATE Client SET public_key = " + txn.quote(key) +
        "WHERE id = " + txn.quote(id) + ";");
    txn.commit();
}

std::vector<Contact> DBServerManager::find_contact(int id, const std::string& name)
{
    std::vector<Contact> contacts;

    pqxx::work txn(*connection_);
    pqxx::result res = txn.exec("SELECT id, nickname, public_key, picture FROM Client \
        WHERE nickname LIKE '%" + name + "%' AND id != " + 
        txn.quote(id) + " ORDER BY nickname LIMIT 10;");

    contacts.reserve(res.size());

    for (const auto& row : res) 
    {
        Contact contact;
        contact.id = row[0].as<int>();
        contact.name = row[1].as<std::string>();
        contact.public_key = row[2].as<std::string>();
        contact.picture = row[3].is_null() ? "" : row[3].as<std::string>();
        contact.chat = -1;

        contacts.push_back(contact);
    }

    return contacts;
}

Contact DBServerManager::get_contact(int id)
{
    pqxx::work txn(*connection_);
    pqxx::result result = txn.exec("SELECT id, nickname, public_key, picture FROM Client \
        WHERE id = " + txn.quote(id) + ";");

    Contact contact;
    if (!result.empty())
    {
        contact.id = result[0]["id"].as<int>();
        contact.name = result[0]["nickname"].as<std::string>();
        contact.public_key = result[0]["public_key"].as<std::string>();
        contact.picture = result[0]["picture"].is_null() ? "" : result[0]["picture"].as<std::string>();
        contact.chat = -1;
    }

    return contact;
}

int DBServerManager::save_chat(const std::string& name, const std::vector<int>& members)
{
    pqxx::work txn(*connection_);

    auto result = txn.exec("INSERT INTO Chat (name, picture) VALUES (" + 
        txn.quote(name) + ", '') RETURNING chat_id;");

    int chat_id = result[0][0].as<int>();

    for(int mem : members)
    {
        txn.exec("INSERT INTO ChatParticipants (client_id, chat_id) VALUES (" +
            txn.quote(mem) + ", " + txn.quote(chat_id) + ");");
    }
    txn.commit();

    return chat_id;
}

void DBServerManager::delete_chat(int chat_id)
{
    pqxx::work txn(*connection_);

    txn.exec("DELETE FROM ChatParticipants WHERE chat_id = " + 
        txn.quote(chat_id) + ";");

    txn.exec("DELETE FROM Chat WHERE chat_id = " +
        txn.quote(chat_id) + ";");

    txn.commit();
}
