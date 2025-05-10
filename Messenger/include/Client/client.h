#ifndef CLIENT_H
#define CLIENT_H

#include "../../../API/protocols.h"
#include "../../../API/resp_codes.h"
#include "../../../API/common_structure.h"
#include "../common_defs.h"
#include "cryptographer.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>
#include <QDebug>
#include <QObject>

using boost::asio::ip::tcp;
using boost::property_tree::ptree;
using ULL = unsigned long long;

class Client : public QObject
{
    Q_OBJECT

public:

    Client(boost::asio::io_service& io, const std::string& ip, int port);

    void connect();

private:

    void get_auth_resp();

    void read();

    boost::asio::const_buffer build_out_msg(const Message& msg);

    void build_in_msg(Message& msg);

    void process_system_msg_respond(const std::vector<QString>& data);

public slots:

    void log_in_user(bool log_in, const std::string& nickname, const std::string& password);

    void write(const Message& msg);

    void send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data);

signals:

    void auth_resp(SERVER_RESP_CODES resp, int id);

    void receive_msg(const Message& msg);

    void send_msg(const Message& msg);

    void list_of_contacts(const QString& name, std::vector<Contact>& contacts);

private:

    boost::asio::io_service& io_;

    std::string server_ip_;
    int port_;
    int id_;

    tcp::socket socket_;
    boost::array<char, 2000> recv_buffer_;

    ptree tree_;
};

#endif // CLIENT_H
