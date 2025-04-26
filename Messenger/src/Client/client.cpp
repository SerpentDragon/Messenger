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

void Client::send_user_data(bool log_in, const std::string& nickname, const std::string& password)
{
    tree_.put(USER_DATA::log_in, log_in);
    tree_.put(USER_DATA::nickname, nickname);
    tree_.put(USER_DATA::password, Cryptographer::hash(password));

    std::ostringstream oss;
    boost::property_tree::write_xml(oss, tree_);

    socket_.send(boost::asio::buffer(oss.str()));

    tree_.clear();

    qDebug() << __func__ << '\n';
}

SERVER_RESP_CODES Client::get_auth_resp()
{
    qDebug() << __func__ << '\n';

    try
    {
        socket_.receive(boost::asio::buffer(recv_buffer_));

        std::string resp = recv_buffer_.data();

        std::stringstream ss(resp);
        read_xml(ss, tree_);

        int resp_code = tree_.get<int>(SERVER_RESPONSE::status);

        tree_.clear();

        return int2SRV_RSP_CD(resp_code);
    }
    catch(...)
    {
        return SERVER_RESP_CODES::ERROR;
    }
}

void Client::start_read()
{
    socket_.async_receive(boost::asio::buffer(recv_buffer_),
        [this](const boost::system::error_code& error, size_t)
        {

        });
}
