#ifndef CLIENT_H
#define CLIENT_H

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>

#include <QDebug>

#include "../../../API/protocols.h"
#include "../../../API/resp_codes.h"
#include "cryptographer.h"

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

class Client
{
public:

    Client(boost::asio::io_service& io, const std::string& ip, int port);

    void connect();

    void send_user_data(bool log_in, const std::string& nickname, const std::string& password);

    SERVER_RESP_CODES get_auth_resp();

private:

    void start_read();

private:

    boost::asio::io_service& io_;

    std::string server_ip_;
    int port_;

    tcp::socket socket_;
    boost::array<char, 2100> recv_buffer_;

    ptree tree_;
};

#endif // CLIENT_H
