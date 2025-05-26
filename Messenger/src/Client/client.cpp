#include "../../include/Client/client.h"

#include <iostream>

Client::Client(boost::asio::io_service& io, const std::string& ip, int port)
    : io_(io), server_ip_(ip), port_(port), socket_(io_), id_(-1) {}

Client::~Client()
{
    socket_.close();
}

void Client::connect()
{
    tcp::resolver resolver(io_);
    tcp::resolver::query query(server_ip_, std::to_string(port_));
    tcp::resolver::results_type endpoints = resolver.resolve(query);

    socket_.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip_), port_));

    std::fill(recv_buffer_.begin(), recv_buffer_.end(), 0);
    socket_.receive(boost::asio::buffer(recv_buffer_));

    server_public_key_ = recv_buffer_.data();

    qDebug() << "SERVER PUBLIC KEY:\n" << server_public_key_ << "\n\n";

    qDebug() << __func__ << '\n';
}

void Client::send_public_key(const std::string& key)
{
    qDebug() << "MY PUBLIC KEY:\n" << key << "\n\n";
    socket_.send(boost::asio::buffer(key));
}

void Client::start_read()
{
    read();
}

void Client::read()
{
    qDebug() << "WAIT FOR A MESSAGE\n";
    boost::asio::async_read_until(socket_, stream_buf_, msg_end,
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec)
                {
                    qDebug() << "RECEIVED MESSAGE!!!!!" << bytes_transferred << "bytes\n";

                    std::fill(recv_buffer_.begin(), recv_buffer_.end(), 0);
                    std::size_t len = bytes_transferred < max_msg_length ?
                                          bytes_transferred : max_msg_length;

                    qDebug() << "PREPARE TO EXTRACT\n";

                    std::istream is(&stream_buf_);
                    is.read(recv_buffer_.data(), len - strlen(msg_end));
                    stream_buf_.consume(strlen(msg_end));

                    qDebug() << "EXTRACTED " << len - strlen(msg_end) << '\n';

                    std::vector<uint8_t> data(recv_buffer_.begin(), recv_buffer_.begin() + len - strlen(msg_end));
                    if (data[0] == 0) data.erase(data.begin());

                    qDebug() << "DATA IS FORMED\n";

                    qDebug() << "DATA:" << data.size();
                    for(int c : data) qDebug() << (int)c << ' ';
                    qDebug() << "\nDATA\n\n";

                    auto result = Cryptographer::get_cryptographer()->decrypt_AES(data);

                    qDebug() << "RESULT: " << result << '\n';

                    process_in_msg(result);

                    read();
                }
            });
}

void Client::process_in_msg(std::string& message)
{
    if (message.find(encryption) != std::string::npos)
    {
        qDebug() << "ENCRYPTION LEVEL\n";

        message = message.substr(message.find(encryption_close_tag) + strlen(encryption_close_tag));

        qDebug() << "MESSAGE: " << message << '\n';

        std::vector<uint8_t> data(message.begin(), message.end());
        if (data[0] == 0) data.erase(data.begin());

        message = Cryptographer::get_cryptographer()->decrypt_AES(data);
    }

    tree_.clear();

    qDebug() << "HERE\n";

    std::stringstream ss(message);

    qDebug() << ss.str() << '\n';

    read_xml(ss, tree_);

    qDebug() << __func__ << ": " << ss.str() << '\n';

    auto opt = tree_.get_optional<int>(SERVER_RESPONSE::status);
    if (opt)
    {
        qDebug() << "GET_AUTH_RESP\n";
        get_auth_resp();
        return;
    }

    opt = tree_.get_optional<bool>(MSG_TAGS::system);
    if (opt)
    {
        if (*opt)
        {
            qDebug() << "PROCESS_SYSTEM_MSG_RESPOND\n";
            process_system_msg_respond();
            return;
        }
    }

    qDebug() << "USUAL MESSAGE\n";

    SocketMessage msg;
    build_in_msg(msg);

    if (!client_public_keys_.contains(msg.sender))
    {
        send_system_msg(SYSTEM_MSG::GET_CONTACT, { QString::number(msg.sender) });
    }

    emit receive_msg(msg);
}

void Client::get_auth_resp()
{
    int resp_code = SERVER_RESP_CODES::ERROR;
    int id = -1;

    resp_code = tree_.get<int>(SERVER_RESPONSE::status);
    id = tree_.get<int>(SERVER_RESPONSE::id);
    id_ = id;

    qDebug() << "LOGIN RESP: " << resp_code << ' ' << id << '\n';

    emit auth_resp(int2SRV_RSP_CD(resp_code), id);
}

void Client::process_system_msg_respond()
{
    switch(tree_.get<int>(SYSTEM_MSG_DATA::cmd))
    {
    case SYSTEM_MSG::LOAD_RSA_KEY:
    {
        break;
    }
    case SYSTEM_MSG::FIND_CONTACT:
    {
        std::vector<Contact> contacts;

        try
        {
            for(const auto& tag : tree_.get_child(SYSTEM_MSG_DATA::data))
            {
                Contact cn = Contact::deserialize(tag.second.data());
                cn.saved_in_db = false;

                contacts.emplace_back(cn);
            }
        }
        catch (const std::exception& ex)
        {
            qDebug() << ex.what() << '\n';
            contacts = {};
        }

        emit list_of_contacts(addl_data[0].toStdString(), contacts);

        break;
    }
    case SYSTEM_MSG::GET_CONTACT:
    {
        Contact cn = Contact::deserialize(tree_.get<std::string>(SYSTEM_MSG_DATA::contact));
        cn.saved_in_db = true;

        client_public_keys_.insert({ cn.id, cn.public_key });

        emit add_contact(cn);

        break;
    }
    case SYSTEM_MSG::NEW_GROUP_CHAT:
    {
        int chat_id;
        qint64 chat_time;
        std::string name;
        std::vector<int> members;

        try
        {
            for(const auto& tag : tree_.get_child(SYSTEM_MSG_DATA::data))
            {
                if (tag.first == "chat_id")
                {
                    chat_id = tree_.get<int>(SYSTEM_MSG_DATA::chat_id);
                }
                else if (tag.first == "chat_name")
                {
                    name = tree_.get<std::string>(SYSTEM_MSG_DATA::chat_name);
                }
                else if (tag.first == "chat_time")
                {
                    chat_time = tree_.get<ULL>(SYSTEM_MSG_DATA::chat_time);
                }
                else if (tag.first == "contact")
                {
                    std::string condata = tag.second.data();
                    qDebug() << "CONDATA:" << condata;
                    Contact cn = Contact::deserialize(condata);
                    cn.saved_in_db = true;
                    qDebug() << "ID:" << cn.id;

                    members.emplace_back(cn.id);

                    if (cn.id != id_)
                    {
                        qDebug() << __func__ << "ADD CONTACT" << cn.id;

                        qDebug() << "SAVE PUBLIC KEY FOR" << cn.id << ":" << cn.public_key << '\n';
                        client_public_keys_.insert({ cn.id, cn.public_key });

                        emit add_contact(cn);
                        emit save_contact(cn);
                    }
                }
            }

            emit add_new_chat(chat_id, name, chat_time, members);
        }
        catch(const std::exception& ex)
        {
            qDebug() << "EXCEPTION: " << ex.what() << '\n';
        }

        break;
    }
    case SYSTEM_MSG::GET_REMOTE_PEER_IP:
    {
        std::string remote_ip = tree_.get<std::string>(SYSTEM_MSG_DATA::ip);
        int contact_id = tree_.get<int>(SYSTEM_MSG_DATA::contact);

        if (remote_ip != "-1")
        {
            int local_port = tree_.get<int>(SYSTEM_MSG_DATA::local_port);
            int remote_port = tree_.get<int>(SYSTEM_MSG_DATA::remote_port);

            qDebug() << remote_ip << contact_id << local_port << remote_port;

            set_P2P_status(contact_id, P2P_CONNECTION_TYPE::TRUE,
                           P2P_CONNECTION_STATUS::SUCCESSFUL);
            create_P2P_connection(contact_id, local_port, remote_ip, remote_port);
        }
        else set_P2P_status(contact_id, P2P_CONNECTION_TYPE::FALSE,
                           P2P_CONNECTION_STATUS::DISCONNECTED);

        break;
    }
    case SYSTEM_MSG::CLOSE_P2P_CONNECTION:
    {
        int contact_id = tree_.get<int>(SYSTEM_MSG_DATA::contact);
        close_p2p_connection(contact_id);
        break;
    }
    }

    addl_data.clear();
}

boost::asio::const_buffer Client::build_out_msg(const SocketMessage& msg)
{
    tree_.clear();

    tree_.put(MSG_TAGS::system, msg.system);
    tree_.put(MSG_TAGS::sender, msg.sender);
    for(int recv : msg.receiver)
    {
        tree_.add(MSG_TAGS::receiver, recv);
    }
    tree_.put(MSG_TAGS::text, msg.text);
    tree_.put(MSG_TAGS::timestamp, msg.timestamp);
    tree_.put(MSG_TAGS::chat, msg.chat);

    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    qDebug() << "OUT MSG: " << ss.str() << '\n';

    return boost::asio::buffer(ss.str());
}

void Client::build_in_msg(SocketMessage& msg)
{
    msg.system = false;
    msg.sender = tree_.get<int>(MSG_TAGS::sender);
    msg.receiver = { id_ };
    msg.text = tree_.get<std::string>(MSG_TAGS::text);
    msg.timestamp = tree_.get<ULL>(MSG_TAGS::timestamp);
    msg.chat = tree_.get<int>(MSG_TAGS::chat);
    msg.vanishing = tree_.get<bool>(MSG_TAGS::vanishing);
}

void Client::create_P2P_connection(int id, int local_port, const std::string &remote_ip, int remote_port)
{
    qDebug() << local_port << remote_ip << remote_port << '\n';

    auto p2p_connector = std::make_unique<P2PConnector>(io_, id_, id, local_port, remote_ip,
                                                        remote_port, client_public_keys_.at(id));
    auto [it, _] = p2p_connections_.insert({ id, std::move(p2p_connector) });

    p2p_connections_[id]->start_p2p_connection();
    QObject::connect(&*it->second, &P2PConnector::receive_msg, this, &Client::receive_p2p_msg);
    QObject::connect(&*it->second, &P2PConnector::close, this, &Client::close_p2p_connection);
    QObject::connect(&*it->second, &P2PConnector::p2p_connection_failed, this, &Client::p2p_connection_failed);
}

void Client::log_in_user(bool log_in, const std::string& nickname, const std::string& password)
{
    tree_.clear();

    tree_.put(USER_DATA::log_in, log_in);
    tree_.put(USER_DATA::nickname, nickname);
    tree_.put(USER_DATA::password, Cryptographer::hash(QString::fromStdString(password)));

    std::ostringstream oss;
    boost::property_tree::write_xml(oss, tree_);

    qDebug() << "SEND CRED: " << oss.str() << '\n';

    auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(oss.str(), server_public_key_);

    std::string enc(encrypted.begin(), encrypted.end());
    qDebug() << "ENCRYPTED CRED: " << enc << '\n';

    socket_.send(boost::asio::buffer(encrypted));
}

void Client::write(SocketMessage& msg)
{
    if (msg.p2p != P2P_CONNECTION_TYPE::TRUE)
    {
        msg.sender = id_;

        for(int i = 0; i < msg.receiver.size(); i++)
        {
            if (msg.receiver[i] == id_) continue;

            tree_.clear();

            tree_.put(MSG_TAGS::system, false);
            tree_.put(MSG_TAGS::sender, msg.sender);
            tree_.put(MSG_TAGS::receiver, msg.receiver[i]);
            tree_.put(MSG_TAGS::text, msg.text);
            tree_.put(MSG_TAGS::timestamp, msg.timestamp);
            tree_.put(MSG_TAGS::chat, msg.chat);
            tree_.put(MSG_TAGS::vanishing, msg.vanishing);

            std::stringstream ss;
            boost::property_tree::write_xml(ss, tree_);

            qDebug() << "send: " << ss.str() << '\n';

            qDebug() << "CLIENT ID:" << msg.receiver[i];

            qDebug() << "USED PUBLIC KEY:" << client_public_keys_[msg.receiver[i]];

            auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(ss.str(),
                                                                             client_public_keys_[msg.receiver[i]]);

            if (msg.p2p == P2P_CONNECTION_TYPE::SERVER)
            {
                std::string encrypt_data = encryption + std::string("true") + encryption_close_tag;
                encrypted.insert(encrypted.begin(), encrypt_data.begin(), encrypt_data.end());

                std::string encrypt_twice(encrypted.begin(), encrypted.end());
                encrypted = Cryptographer::get_cryptographer()->encrypt_AES(encrypt_twice,
                                                                            client_public_keys_[msg.receiver[i]]);
            }

            std::string recv_data = recv_open_tag + std::to_string(msg.receiver[i]) + recv_close_tag;
            encrypted.insert(encrypted.begin(), recv_data.begin(), recv_data.end());
            encrypted.insert(encrypted.end(), std::begin(msg_end), std::end(msg_end) - 1);

            socket_.send(boost::asio::buffer(encrypted));
        }

        for(int recv : msg.receiver) qDebug() << "WRITE RECV:" << recv;
    }
    else
    {
        int receiver = msg.receiver[0];
        if (p2p_connections_.contains(receiver))
        {
            p2p_connections_[receiver]->send(msg);
        }
    }

    emit send_msg(msg);
}

void Client::send_system_msg(SYSTEM_MSG type, const std::vector<QString>& data)
{
    tree_.clear();
    tree_.put(SYSTEM_MSG_DATA::system, true);
    tree_.put(SYSTEM_MSG_DATA::cmd, type);
    tree_.put(SYSTEM_MSG_DATA::sender, id_);

    switch(type)
    {
    case SYSTEM_MSG::LOAD_RSA_KEY:
    {
        tree_.put(SYSTEM_MSG_DATA::key, data[0].toStdString());
        break;
    }
    case SYSTEM_MSG::FIND_CONTACT:
    case SYSTEM_MSG::GET_CONTACT:
    case SYSTEM_MSG::GET_REMOTE_PEER_IP:
    {
        tree_.put(SYSTEM_MSG_DATA::contact, data[0].toStdString());
        break;
    }
    case SYSTEM_MSG::NEW_GROUP_CHAT:
    {
        tree_.put(SYSTEM_MSG_DATA::chat_name, data[0].toStdString());
        tree_.put(SYSTEM_MSG_DATA::chat_time, data[1].toStdString());

        for(int i = 2; i < data.size(); i++)
        {
            qDebug() << data[i] << '\n';
            tree_.add(SYSTEM_MSG_DATA::chat_member, data[i].toStdString());
        }

        break;
    }
    case SYSTEM_MSG::CLOSE_P2P_CONNECTION:
    {
        tree_.put(SYSTEM_MSG_DATA::contact, data[0].toStdString());
        break;
    }
    default: return;
    }

    addl_data = data;

    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(ss.str(), server_public_key_);
    encrypted.insert(encrypted.end(), std::begin(msg_end), std::end(msg_end) - 1);

    qDebug() << "PROCESS_MSG: " << ss.str() << '\n';

    socket_.send(boost::asio::buffer(encrypted));
}

void Client::save_public_key(int id, const std::string& public_key)
{
    qDebug() << "CLIENT PUBLIC KEY\n" << public_key << "\n\n";
    client_public_keys_.insert({ id, public_key });
}

void Client::update_keys_cash(const std::unordered_map<int, std::string>& cash)
{
    client_public_keys_ = cash;
}

void Client::new_chat(const QString &name, qint64 time, const std::vector<int> members)
{
    std::vector<QString> data;
    data.emplace_back(name);
    data.emplace_back(QString::number(time));
    data.emplace_back(QString::number(id_));

    qDebug() << "CLIENT CHAT TIME:" << QString::number(time);

    for(int mem : members)
    {
        qDebug() << "MEMBER: " << mem << '\n';
        data.emplace_back(QString::number(mem));
    }

    send_system_msg(SYSTEM_MSG::NEW_GROUP_CHAT, data);
}

void Client::establish_p2p_connection(int id)
{
    if (p2p_connections_.contains(id)) return;

    qDebug() << "SEND SYSTEM MSG TO ESTABLISH P2P";

    send_system_msg(SYSTEM_MSG::GET_REMOTE_PEER_IP, { QString::number(id) });
}

void Client::close_p2p_connection(int id)
{
    qDebug() << __func__ << id << '\n';

    if (p2p_connections_.contains(id))
    {
        if (!p2p_connections_[id]->close_p2p_connection())
        {
            send_system_msg(SYSTEM_MSG::CLOSE_P2P_CONNECTION, { QString::number(id) });
        }
        p2p_connections_.erase(id);

        qDebug() << "P2P CONNECTION IS CLOSED!\n";

        emit set_P2P_status(id, P2P_CONNECTION_TYPE::FALSE,
                            P2P_CONNECTION_STATUS::DISCONNECTED);
    }
    qDebug() << "leave" << __func__ << '\n';
}

void Client::receive_p2p_msg(const SocketMessage& msg)
{
    emit receive_msg(msg);
}

void Client::p2p_connection_failed(int id)
{
    qDebug() << __func__ << id << '\n';

    if (p2p_connections_.contains(id))
    {
        p2p_connections_[id]->close_p2p_connection();
        // p2p_connections_.erase(id);

        qDebug() << "P2P TRHROUGH THE SERVER!\n";

        emit set_P2P_status(id, P2P_CONNECTION_TYPE::SERVER,
                             P2P_CONNECTION_STATUS::SERVER_FALLBACK);
    }

    qDebug() << "leave" << __func__ << '\n';
}
