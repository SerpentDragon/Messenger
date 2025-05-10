#include "database.h"

#include <iostream>

DB_Server_Manager::~DB_Server_Manager()
{
    connection_->close();
}

void DB_Server_Manager::connect(const std::string& address)
{
    connection_ = std::make_unique<pqxx::connection>(address);

    if(!connection_->is_open())
    {
        throw std::runtime_error("Error connecting to " + address + "!\n");
    }
}

std::string DB_Server_Manager::log_in_client(int& id, const std::string& nickname)
{
    pqxx::work txn(*connection_);
    pqxx::result res = txn.exec("SELECT id, password FROM Client WHERE nickname = " + txn.quote(nickname));

    id = res.empty() ? -1 : res[0][0].as<int>();

    return res.empty() ? "" : res[0][1].as<std::string>();
}

bool DB_Server_Manager::sign_up_client(int& id, const std::string& nickname, const std::string& password)
{
    pqxx::transaction txn(*connection_);

    try
    {
        auto result = txn.exec("INSERT INTO Client (nickname, password, picture) VALUES (" +
                 txn.quote(nickname) + ", " + 
                 txn.quote(password) + ", " + 
                 txn.quote("") + ") RETURNING id;");

        txn.commit();

        id = result[0][0].as<int>();

        std::cout << "DONE: " << id << '\n';
    }
    catch(const std::exception& ex)
    {
        std::cout << ex.what() << '\n';
        id = -1;
        return false;
    }

    return true;
}

std::vector<Contact> DB_Server_Manager::find_contact(const std::string& name)
{
    std::vector<Contact> contacts;

    pqxx::work txn(*connection_);
    pqxx::result res = txn.exec("SELECT id, nickname, picture FROM Client \
        WHERE nickname LIKE '%" + name + "%' ORDER BY nickname LIMIT 10;");

    contacts.reserve(res.size());

    for (const auto& row : res) 
    {
        Contact contact;
        contact.id = row[0].as<int>();
        contact.name = row[1].as<std::string>();
        contact.picture = row[2].as<std::string>();
        contact.chat = -1;

        std::cout << contact.id << ' ';

        contacts.push_back(contact);
    }

    std::cout << '\n';

    return contacts;
}