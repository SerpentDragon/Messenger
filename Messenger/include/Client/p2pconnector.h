#ifndef P2PCONNECTOR_H
#define P2PCONNECTOR_H

#include "../../../API/protocols.h"
#include "../common_defs.h"
#include "cryptographer.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <QObject>
#include <string>

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

class P2PConnector : public QObject
{
    Q_OBJECT

public:

    P2PConnector(boost::asio::io_service& io, int id, int recv_id,
                 int local_port, const std::string& remote_ip,
                 int remote_port, const std::string& public_key);

    void start_p2p_connection();

    void close_p2p_connection();

    void send(SocketMessage& msg);

private:

    void start_accept();

    void connect_to_peer();

    void start_read();

    void process_msg(const std::string& msg);

    void process_system_msg(const std::string& msg);

    void process_user_msg(const std::string& msg);

    void write_to_socket();

signals:

    void receive_msg(const SocketMessage& msg);

    void close(int id);

private:

    boost::asio::io_service& io_;

    std::string remote_ip_;
    int local_port_;
    int remote_port_;

    tcp::acceptor acceptor_;
    tcp::socket socket_;
    bool connected_;

    int id_;
    int recv_id_;

    ptree tree_;

    std::string client_public_key_;
};

#endif // P2PCONNECTOR_H
