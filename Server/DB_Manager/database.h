#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <pqxx/pqxx>

#include "../../API/common_structure.h"

class DB_Server_Manager
{
public:

    DB_Server_Manager() = default;

    ~DB_Server_Manager();

    void connect(const std::string& address);

    std::string log_in_client(int& id, const std::string& nickname);

    bool sign_up_client(int& id, const std::string& nickname, const std::string& password);

    std::vector<Contact> find_contact(const std::string& name);

private:

    std::unique_ptr<pqxx::connection> connection_;
};

#endif // DATABASE_H
