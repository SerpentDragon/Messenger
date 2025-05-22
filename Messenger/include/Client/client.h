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

    ~Client();

    void connect();

    void send_public_key(const std::string& key);

    void start_read();

private:

    void read();

    void process_in_msg(const std::string& message);

    void get_auth_resp();

    void process_system_msg_respond();
    
    boost::asio::const_buffer build_out_msg(const SocketMessage& msg);
    
    void build_in_msg(SocketMessage& msg);

public slots:

    void log_in_user(bool log_in, const std::string& nickname, const std::string& password);
    
    void write(SocketMessage& msg);

    void send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data);

    void save_public_key(int id, const std::string& public_key);

    void update_keys_cash(const std::unordered_map<int, std::string>& cash);

    void new_chat(const QString& name, qint64 time, const std::vector<int> members);

signals:

    void auth_resp(SERVER_RESP_CODES resp, int id);
    
    void receive_msg(const SocketMessage& msg);
    
    void send_msg(const SocketMessage& msg);

    void list_of_contacts(const std::string& name, const std::vector<Contact>& contacts);

    void add_contact(const Contact& contact);

    void add_new_chat(int id, const std::string& name, const std::vector<int>& members);

    void save_contact(const Contact& contact);

private:

    boost::asio::io_service& io_;

    std::string server_ip_;
    int port_;
    int id_;

    tcp::socket socket_;
    boost::asio::streambuf stream_buf_;
    boost::array<char, max_msg_length> recv_buffer_;

    ptree tree_;
    std::vector<QString> addl_data;

    std::string server_public_key_;

    std::unordered_map<int, std::string> client_public_keys_;
};

#endif // CLIENT_H
