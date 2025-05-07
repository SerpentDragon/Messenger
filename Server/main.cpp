#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <map>
#include <pqxx/pqxx>
#include <sstream>

#include "DB_Manager/database.h"
#include "../API/resp_codes.h"
#include "../API/protocols.h"

#include <iostream>

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

class Server
{
public:

    Server(boost::asio::io_service& io, const int port) :
        io_(io), acceptor_(io_, tcp::endpoint(tcp::v4(), port))
    {
        db_manager_.connect("dbname=messenger_server user=dmitry password=1223684DS hostaddr=127.0.0.1 port=5432");

        start_accept();
    }

private:

    void start_accept()
    {
        std::cout << __func__ << '\n';

        boost::shared_ptr<tcp::socket> new_socket(new tcp::socket(io_));

        acceptor_.async_accept(*new_socket,
            [this, new_socket](const boost::system::error_code& ec)
            {
                if (!ec) handle_accept(new_socket);
                else std::cerr << "Error accepting new connection: " << ec.message() << '\n';
            });
    }

    void handle_accept(boost::shared_ptr<tcp::socket> new_socket)
    {
        std::cout << __func__ << '\n';

        boost::shared_ptr<boost::array<char, msg_length_>> data(new boost::array<char, msg_length_>);
        memset(data->data(), 0, msg_length_);
        
        new_socket->async_receive(boost::asio::buffer(*data),
            [this, data, new_socket](const boost::system::error_code& ec, size_t)
            {
                std::cout << data->data() << '\n';
                if (!ec) add_client(new_socket, data->data());
                else std::cerr << "Error receiving data: " << ec.message() << '\n';
            });

        start_accept();
    }

    void add_client(boost::shared_ptr<tcp::socket> new_socket, const std::string& data)
    {
        std::cout << __func__ << '\n';

        std::stringstream ss(data);
        read_xml(ss, tree_);
        
        bool log_in = tree_.get<bool>(USER_DATA::log_in);
        std::string nickname = tree_.get<std::string>(USER_DATA::nickname);
        std::string password = tree_.get<std::string>(USER_DATA::password);
        tree_.clear();

        int id;
        SERVER_RESP_CODES resp = SERVER_RESP_CODES::ERROR;

        std::cout << log_in << ' ' << nickname << ' ' << password << '\n';

        if (log_in)
        {
            std::string password_from_db = db_manager_.log_in_client(id, nickname);

            std::cout << "psswd_frm_db: " << password_from_db << '\n';

            if (password_from_db == password)
            {
                resp = SERVER_RESP_CODES::OK;
            }

            if (password_from_db.empty())
            {
                resp = SERVER_RESP_CODES::NO_USER_NICKNAME;
            }

            if (password_from_db != password)
            {
                resp = SERVER_RESP_CODES::WRONG_NICKNAME_PSSWD;
            }
        }
        else
        {
            if (db_manager_.sign_up_client(id, nickname, password))
            {
                resp = SERVER_RESP_CODES::OK;
            }
            else
            {
                resp = SERVER_RESP_CODES::NICKNAME_TAKEN;
            }   
        }

        build_response(tree_, resp, id);

        std::ostringstream oss;
        boost::property_tree::write_xml(oss, tree_);

        new_socket->send(boost::asio::buffer(oss.str()), 0);

        tree_.clear();

        if (resp != SERVER_RESP_CODES::OK)
        {
            std::cout << "Try again\n";
            handle_accept(new_socket);
        }

        online_clients_[id] = new_socket;

        start_read(id);
    }

    void start_read(int id)
    {
        // auto socket = online_clients_[id];

        // boost::shared_ptr<boost::array<char, msg_length_>> data(new boost::array<char, msg_length_>);
        // memset(data->data(), 0, msg_length_);

        // socket->async_receive(boost::asio::buffer(*data), 
        //     [this, data, socket](const boost::system::error_code& ec, size_t)
        //     {
        //         if (!ec)
        //         {
        //             std::stringstream ss(data->data());
        //             read_xml(ss, tree_);

        //             int receiver = tree_.get<int>(MSG_TAGS::receiver);

        //             tree_.clear();

        //             online_clients_[receiver]->send(*data);
        //         }
        //     });
    }

private:

    static void build_response(ptree& tree, SERVER_RESP_CODES rc, int id)
    {
        tree.put(SERVER_RESPONSE::status, rc);
        tree.put(SERVER_RESPONSE::id, id);
    }

private:

    boost::asio::io_service& io_;
    tcp::acceptor acceptor_;

    std::map<int, boost::shared_ptr<tcp::socket>> online_clients_;

    DB_Server_Manager db_manager_;
    ptree tree_;

    static constexpr int msg_length_ = 2000;
};

int main()
{
    try
    {
        boost::asio::io_service io;

        Server server(io, 1545);

        io.run();
    }
    catch(const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
    }

    return 0;
}
