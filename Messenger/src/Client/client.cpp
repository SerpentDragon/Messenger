#include "../../include/Client/client.h"

Client::Client(boost::asio::io_service& io, const std::string& ip, int port)
    : io_(io), server_ip_(ip), port_(port), socket_(io_), id_(-1)
{
}

void Client::connect()
{
    tcp::resolver resolver(io_);
    tcp::resolver::query query(server_ip_, std::to_string(port_));
    tcp::resolver::results_type endpoints = resolver.resolve(query);

    socket_.connect(tcp::endpoint(boost::asio::ip::make_address(server_ip_), port_));

    qDebug() << __func__ << '\n';
}

void Client::get_auth_resp()
{
    qDebug() << __func__ << '\n';

    int resp_code = SERVER_RESP_CODES::ERROR;
    int id = -1;

    try
    {
        std::fill(recv_buffer_.begin(), recv_buffer_.end(), 0);
        socket_.receive(boost::asio::buffer(recv_buffer_));

        tree_.clear();

        std::stringstream ss(recv_buffer_.data());
        read_xml(ss, tree_);

        resp_code = tree_.get<int>(SERVER_RESPONSE::status);
        id = tree_.get<int>(SERVER_RESPONSE::id);
        id_ = id;

        qDebug() << "LOGIN RESP: " << resp_code << ' ' << id << '\n';
    }
    catch(...)
    {
        resp_code = SERVER_RESP_CODES::ERROR;
    }

    emit auth_resp(int2SRV_RSP_CD(resp_code), id);
}

void Client::read()
{
    socket_.async_receive(boost::asio::buffer(recv_buffer_),
        [this](const boost::system::error_code& ec, size_t)
        {
            if (!ec)
            {
                Message msg;

                build_in_msg(msg);

                emit receive_msg(msg);

                read();
            }
        }
    );
}

boost::asio::const_buffer Client::build_out_msg(const Message &msg)
{
    tree_.clear();

    tree_.put(MSG_TAGS::system, msg.system);
    tree_.put(MSG_TAGS::sender, msg.sender);
    tree_.put(MSG_TAGS::receiver, msg.receiver);
    tree_.put(MSG_TAGS::text, msg.text);
    tree_.put(MSG_TAGS::timestamp, msg.timestamp);
    tree_.put(MSG_TAGS::chat, msg.chat);

    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    return boost::asio::buffer(ss.str());
}

void Client::build_in_msg(Message& msg)
{
    tree_.clear();

    std::stringstream ss(recv_buffer_.data());
    read_xml(ss, tree_);

    msg.system = tree_.get<bool>(MSG_TAGS::system);
    msg.sender = tree_.get<int>(MSG_TAGS::sender);
    msg.receiver = tree_.get<int>(MSG_TAGS::receiver);
    msg.text = tree_.get<std::string>(MSG_TAGS::text);
    msg.timestamp = tree_.get<ULL>(MSG_TAGS::timestamp);
    msg.chat = tree_.get<int>(MSG_TAGS::chat);
}

void Client::process_system_msg_respond(const std::vector<QString>& data)
{
    std::fill(recv_buffer_.begin(), recv_buffer_.end(), 0);
    socket_.receive(boost::asio::buffer(recv_buffer_));

    tree_.clear();

    std::stringstream ss(recv_buffer_.data());
    read_xml(ss, tree_);

    qDebug() << ss.str() << '\n';

    switch(tree_.get<int>(SYSTEM_MSG_DATA::cmd))
    {
    case SYSTEM_MSG::FIND_CONTACT:
    {
        qDebug() << "FIND\n";

        std::vector<Contact> contacts;

        for(const auto& tag : tree_.get_child(SYSTEM_MSG_DATA::data))
        {
            qDebug() << tag.second.data() << '\n';
            contacts.emplace_back(Contact::deserialize(tag.second.data()));
        }

        emit list_of_contacts(data[0], contacts);

        break;
    }
    }
}

void Client::log_in_user(bool log_in, const std::string& nickname, const std::string& password)
{
    tree_.clear();

    tree_.put(USER_DATA::log_in, log_in);
    tree_.put(USER_DATA::nickname, nickname);
    tree_.put(USER_DATA::password, Cryptographer::hash(password));

    std::ostringstream oss;
    boost::property_tree::write_xml(oss, tree_);

    socket_.send(boost::asio::buffer(oss.str()));

    qDebug() << __func__ << '\n';

    get_auth_resp();
}

void Client::write(const Message& msg)
{
    socket_.send(build_out_msg(msg));

    // emit

    tree_.clear();
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

    std::stringstream ss;
    boost::property_tree::write_xml(ss, tree_);

    qDebug() << ss.str() << '\n';

    qDebug() << "SYS_MSG_TYPE: " << type << '\n';

    socket_.send(boost::asio::buffer(ss.str()));

    qDebug() << "SYS_MSG_TYPE SENT\n";

    process_system_msg_respond(data);
}
