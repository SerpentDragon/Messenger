#include "../../include/Client/client.h"

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

    qDebug() << __func__ << '\n';
}

void Client::start_read()
{
    read();
}

void Client::read()
{
    std::fill(recv_buffer_.begin(), recv_buffer_.end(), 0);
    boost::asio::async_read_until(socket_, stream_buf_, msg_end,
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec)
                {
                    std::size_t len = bytes_transferred < max_msg_length ?
                                          bytes_transferred : max_msg_length;

                    std::istream is(&stream_buf_);
                    is.read(recv_buffer_.data(), len - strlen(msg_end));
                    stream_buf_.consume(strlen(msg_end));

                    process_in_msg();

                    read();
                }
        });
}

void Client::process_in_msg()
{
    tree_.clear();

    std::stringstream ss(recv_buffer_.data());
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
    case SYSTEM_MSG::FIND_CONTACT:
    {
        std::vector<Contact> contacts;

        for(const auto& tag : tree_.get_child(SYSTEM_MSG_DATA::data))
        {
            Contact cn = Contact::deserialize(tag.second.data());
            cn.saved_in_db = false;

            contacts.emplace_back(cn);
        }

        emit list_of_contacts(addl_data[0].toStdString(), contacts);

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
    // tree_.clear();

    // std::stringstream ss(recv_buffer_.data());
    // qDebug() << __func__ << ' ' << ss.str() << '\n';
    // read_xml(ss, tree_);

    msg.system = tree_.get<bool>(MSG_TAGS::system);
    msg.sender = tree_.get<int>(MSG_TAGS::sender);
    for(const auto& child : tree_.get_child(MSG_TAGS::msg))
    {
        if (child.first == "receiver")
        {
            int val = child.second.get<int>("");
            msg.receiver.emplace_back(val);
        }
    }
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

    socket_.send(boost::asio::buffer(oss.str()));
}

void Client::write(SocketMessage& msg)
{
    msg.sender = id_;

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

    qDebug() << "send: " << ss.str() << '\n';

    socket_.send(boost::asio::buffer(ss.str()));

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
    case SYSTEM_MSG::FIND_CONTACT:
    {
        tree_.put(SYSTEM_MSG_DATA::contact, data[0].toStdString());
        break;
    }
    }

    addl_data = data;

    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    qDebug() << ss.str() << '\n';

    socket_.send(boost::asio::buffer(ss.str()));

    // process_system_msg_respond(data);
}
