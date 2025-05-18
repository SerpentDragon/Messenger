#ifndef DATABASE_H
#define DATABASE_H

#include <memory>
#include <pqxx/pqxx>

#include "../../API/common_structure.h"

class DBServerManager
{
public:

    DBServerManager() = default;

    ~DBServerManager();

    void connect(const std::string& address);

    void save_RSA_keys(const std::pair<std::string, std::string>& keys);

    std::string log_in_client(int& id, const std::string& nickname);

    bool sign_up_client(int& id, const std::string& nickname, const std::string& password);

    void load_RSA_key(int id, const std::string key);

    std::vector<Contact> find_contact(int id, const std::string& name);

    Contact get_contact(int id);

private:

    std::unique_ptr<pqxx::connection> connection_;
};

#endif // DATABASE_H
