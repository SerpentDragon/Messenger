#ifndef DBMANAGER_H
#define DBMANAGER_H

#include "../common_defs.h"

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

private:

    void connect(const std::string& address);

public slots:

    void db_connect(bool log_in, int id, const std::string& nickname);

    void save_msg(const Message& msg);

private:

    std::unique_ptr<pqxx::connection> connection_;

    int id_;
};

#endif // DBMANAGER_H
