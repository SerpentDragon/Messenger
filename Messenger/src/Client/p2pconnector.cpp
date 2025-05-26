#include "../../include/Client/p2pconnector.h"

#include <QDebug>

P2PConnector::P2PConnector(boost::asio::io_service& io, int id, int recv_id,
                           int local_port, const std::string& remote_ip,
                           int remote_port, const std::string& public_key)
    : io_(io), remote_ip_(remote_ip), local_port_(local_port), remote_port_(remote_port),
    acceptor_(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), local_port_)),
    socket_(io), timeout_timer_(io), connected_(false), id_(id), recv_id_(recv_id),
    client_public_key_(public_key)
{
    qDebug() << "P2P PUBLIC KEY:" << client_public_key_;
}

void P2PConnector::start_p2p_connection()
{
    start_accept();

    connect_to_peer();

    timeout_timer_.expires_after(std::chrono::seconds(3));
    timeout_timer_.async_wait([this](const boost::system::error_code& ec) {
        if (!ec && !connected_)
        {
            socket_.close();
            qDebug() << "P2P connection timeout: no incoming or outgoing connection established.";
            emit p2p_connection_failed(recv_id_);
        }
    });
}

bool P2PConnector::close_p2p_connection()
{
    if (!socket_.is_open()) return false;

    tree_.clear();
    tree_.put(MSG_TAGS::system, true);
    tree_.put(MSG_TAGS::sender, id_);
    tree_.put(SYSTEM_MSG_DATA::cmd, SYSTEM_MSG::CLOSE_P2P_CONNECTION);

    write_to_socket();

    socket_.close();

    return true;
}

void P2PConnector::send(SocketMessage &msg)
{
    msg.sender = id_;

    tree_.clear();
    tree_.put(MSG_TAGS::system, false);
    tree_.put(MSG_TAGS::sender, msg.sender);
    tree_.put(MSG_TAGS::receiver, msg.receiver[0]);
    tree_.put(MSG_TAGS::text, msg.text);
    tree_.put(MSG_TAGS::timestamp, msg.timestamp);
    tree_.put(MSG_TAGS::chat, msg.chat);
    tree_.put(MSG_TAGS::vanishing, msg.vanishing);

    qDebug() << "SENT P2P MESSAGE\n";

    write_to_socket();
}

void P2PConnector::start_accept()
{
    acceptor_.async_accept([this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket new_socket) {
        if (!ec && !connected_)
        {
            connected_ = true;
            socket_ = std::move(new_socket);
            qDebug() << "Successfully accepted!\n";
            start_read();
        }
        else
        {
            qDebug() << "Error accepting connection!\n";
        }
    });
}

void P2PConnector::connect_to_peer()
{
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(remote_ip_), remote_port_);
    qDebug() << "Trying to connect to" << remote_ip_ << remote_port_;
    socket_.async_connect(endpoint, [this](const boost::system::error_code& ec) {
        if (!ec && !connected_)
        {
            connected_ = true;
            start_read();
            qDebug() << "Successfully connected!\n";
        }
        else
        {
            qDebug() << "Error connecting to the peer!\n";
        }
    });
};

void P2PConnector::start_read()
{
    if (!socket_.is_open()) return;

    auto buffer = std::make_shared<boost::array<char, max_msg_length>>();
    socket_.async_receive(boost::asio::buffer(*buffer),
        [this, buffer](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (!ec)
            {
                std::vector<uint8_t> data(buffer->begin(), buffer->begin() + bytes_transferred);
                if (data[0] == 0) data.erase(data.begin());

                // qDebug() << "P2P RECV:" << data.size();
                // for(int c : data) qDebug() << (int)c;
                // qDebug() << "\nP2P RECV\n\n";

                auto result = Cryptographer::get_cryptographer()->decrypt_AES(data);

                process_msg(result);

                start_read();
            }
        }
    );
}

void P2PConnector::process_msg(const std::string& msg)
{
    tree_.clear();

    std::stringstream ss(msg);
    read_xml(ss, tree_);

    auto opt = tree_.get_optional<bool>(MSG_TAGS::system);
    if(opt)
    {
        if (*opt)
        {
            process_system_msg(msg);
        }
        else
        {
            process_user_msg(msg);
        }
    }
}

void P2PConnector::process_system_msg(const std::string& msg)
{
    qDebug() << "P2P SYSTEM MSG:" << msg << '\n';
    int cmd = tree_.get<int>(SYSTEM_MSG_DATA::cmd);

    if (cmd == SYSTEM_MSG::CLOSE_P2P_CONNECTION)
    {
        socket_.close();
        emit close(recv_id_);
    }
}

void P2PConnector::process_user_msg(const std::string& msg)
{
    SocketMessage socket_msg;

    socket_msg.system = false;
    socket_msg.sender = tree_.get<int>(MSG_TAGS::sender);
    socket_msg.receiver = { id_ };
    socket_msg.text = tree_.get<std::string>(MSG_TAGS::text);
    socket_msg.timestamp = tree_.get<ULL>(MSG_TAGS::timestamp);
    socket_msg.chat = tree_.get<int>(MSG_TAGS::chat);
    socket_msg.vanishing = tree_.get<bool>(MSG_TAGS::vanishing);

    qDebug() << "RECEIVED P2P Message\n";

    emit receive_msg(socket_msg);
}

void P2PConnector::write_to_socket()
{
    if (!socket_.is_open()) return;
    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(ss.str(), client_public_key_);

    // qDebug() << "P2P DATA:" << encrypted.size();
    // for(int c : encrypted) qDebug() << (int)c;
    // qDebug() << "\nP2P DATA\n\n";

    socket_.send(boost::asio::buffer(encrypted));
}
