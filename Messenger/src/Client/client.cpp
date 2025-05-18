#include "../../include/Client/client.h"

#include <iostream>

Client::Client(boost::asio::io_service& io, const std::string& ip, int port)
    : io_(io), server_ip_(ip), port_(port), socket_(io_), id_(-1)
{
}

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
                    qDebug() << "RECEIVED MESSAGE!!!!!";

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

                    qDebug() << "DATA:\n";
                    for(int c : data) std::cout << std::dec << (int)c << ' ';
                    qDebug() << "\nDATA\n\n";

                    auto result = Cryptographer::get_cryptographer()->decrypt_AES(data);

                    qDebug() << "RESULT: " << result << '\n';

                    process_in_msg(result);

                    read();
                }
        });
}

void Client::process_in_msg(const std::string& message)
{
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

    emit receive_msg(false, msg);
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

        emit add_contact(cn);

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
    // for(const auto& child : tree_.get_child(MSG_TAGS::msg))
    // {
    //     if (child.first == "receiver")
    //     {
    //         int val = child.second.get<int>("");
    //         msg.receiver.emplace_back(val);
    //     }
    // }
    msg.text = tree_.get<std::string>(MSG_TAGS::text);
    msg.timestamp = tree_.get<ULL>(MSG_TAGS::timestamp);
    msg.chat = tree_.get<int>(MSG_TAGS::chat);
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
    msg.sender = id_;

    for(int i = 0; i < msg.receiver.size(); i++)
    {
        tree_.clear();

        tree_.put(MSG_TAGS::system, msg.system);
        tree_.put(MSG_TAGS::sender, msg.sender);
        //for(int recv : msg.receiver)
        //{
        //    tree_.add(MSG_TAGS::receiver, recv);
        //}
        tree_.put(MSG_TAGS::receiver, msg.receiver[i]);
        tree_.put(MSG_TAGS::text, msg.text);
        tree_.put(MSG_TAGS::timestamp, msg.timestamp);
        tree_.put(MSG_TAGS::chat, msg.chat);

        std::stringstream ss;
        boost::property_tree::write_xml(ss, tree_);

        qDebug() << "send: " << ss.str() << '\n';

        auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(ss.str(), client_public_keys_[msg.receiver[i]]);

        std::string recv_data = recv_open_tag + std::to_string(msg.receiver[i]) + recv_close_tag;
        encrypted.insert(encrypted.begin(), recv_data.begin(), recv_data.end());

        socket_.send(boost::asio::buffer(encrypted));

        if (i == msg.receiver.size() - 1) [[unlikely]] emit send_msg(true, msg);
        else emit send_msg(false, msg);
    }
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
