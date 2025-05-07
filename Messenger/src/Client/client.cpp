#include "../../include/Client/client.h"

Client::Client(boost::asio::io_service& io, const std::string& ip, int port)
    : io_(io), server_ip_(ip), port_(port), socket_(io_)
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

    int resp_code;
    int id;

    try
    {
        socket_.receive(boost::asio::buffer(recv_buffer_));

        resp_code = SERVER_RESP_CODES::ERROR;

        std::string resp = recv_buffer_.data();

        std::stringstream ss(resp);
        read_xml(ss, tree_);

        resp_code = tree_.get<int>(SERVER_RESPONSE::status);
        id = tree_.get<int>(SERVER_RESPONSE::id);

        qDebug() << "resp: " << resp_code << '\n';

        tree_.clear();
    }
    catch(...)
    {
        tree_.clear();
        resp_code = SERVER_RESP_CODES::ERROR;
    }

    emit auth_resp(int2SRV_RSP_CD(resp_code), id);
}

void Client::start_read()
{
    socket_.async_receive(boost::asio::buffer(recv_buffer_),
        [this](const boost::system::error_code& error, size_t)
        {

        });
}

void Client::log_in_user(bool log_in, const std::string& nickname, const std::string& password)
{
    qDebug() << nickname << ' ' << password << '\n';

    tree_.put(USER_DATA::log_in, log_in);
    tree_.put(USER_DATA::nickname, nickname);
    tree_.put(USER_DATA::password, Cryptographer::hash(password));

    std::ostringstream oss;
    boost::property_tree::write_xml(oss, tree_);

    socket_.send(boost::asio::buffer(oss.str()));

    tree_.clear();

    qDebug() << __func__ << '\n';

    get_auth_resp();
}

void Client::send_message(const QString &text)
{
    tree_.put(MSG_TAGS::sender, "");
    tree_.put(MSG_TAGS::receiver, "");
    tree_.put(MSG_TAGS::text, text.toStdString());

    std::ostringstream oss;
    boost::property_tree::write_xml(oss, tree_);

    socket_.send(boost::asio::buffer(oss.str()));

    tree_.clear();
}

