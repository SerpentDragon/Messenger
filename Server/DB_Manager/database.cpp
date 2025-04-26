#include "database.h"

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
    pqxx::result res = txn.exec("SELECT id, password FROM client WHERE nickname = " + txn.quote(nickname));

    id = res.empty() ? -1 : res[0][0].as<int>();

    return res.empty() ? "" : res[0][1].as<std::string>();
}

bool DB_Server_Manager::sign_up_client(int& id, const std::string& nickname, const std::string& password)
{
    pqxx::transaction txn(*connection_);

    try
    {
        auto result = txn.exec("INSERT INTO client (nickname, password) VALUES (" +
                 txn.quote(nickname) + ", " + txn.quote(password) + ") RETURNING id;");

        txn.commit();

        id = result[0][0].as<int>();
    }
    catch(...)
    {
        id = -1;
        return false;
    }

    return true;
}
