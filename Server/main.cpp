#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <pqxx/pqxx>
#include <sstream>
#include <unordered_map>

#include "DB_Manager/database.h"
#include "ServerUtils/messagequeue.h"
#include "../API/resp_codes.h"
#include "../API/protocols.h"
#include "../API/common_structure.h"

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

class Server
{
private:

    using BUFFER = boost::shared_ptr<boost::array<char, max_msg_length>>;
    using SOCKET = boost::shared_ptr<tcp::socket>;

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

        SOCKET new_socket(new tcp::socket(io_));

        acceptor_.async_accept(*new_socket,
            [this, new_socket](const boost::system::error_code& ec)
            {
                if (!ec) handle_accept(new_socket);
                else std::cerr << "Error accepting new connection: " << ec.message() << '\n';
            });
    }

    void handle_accept(SOCKET new_socket)
    {
        std::cout << __func__ << '\n';

        BUFFER data(new boost::array<char, max_msg_length>);
        memset(data->data(), 0, max_msg_length);
        
        new_socket->async_receive(boost::asio::buffer(*data),
            [this, data, new_socket](const boost::system::error_code& ec, size_t)
            {
                std::cout << data->data() << '\n';
                if (!ec) add_client(new_socket, data->data());
                else std::cerr << "Error receiving data: " << ec.message() << '\n';
            });

        start_accept();
    }

    void add_client(SOCKET new_socket, const std::string& data)
    {
        std::cout << __func__ << '\n';

        std::stringstream ss(data);
        read_xml(ss, tree_);

        std::cout << ss.str() << '\n';
        
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
            else if (password_from_db.empty())
            {
                resp = SERVER_RESP_CODES::NO_USER_NICKNAME;
            }
            else if (password_from_db != password)
            {
                resp = SERVER_RESP_CODES::WRONG_NICKNAME_PSSWD;
            }
        }
        else
        {
            if (db_manager_.sign_up_client(id, nickname, password))
            {
                resp = SERVER_RESP_CODES::OK;
                std::cout << "RESP: OK\n";
            }
            else
            {
                resp = SERVER_RESP_CODES::NICKNAME_TAKEN;
            }   
        }

        build_response(tree_, resp, id);

        std::ostringstream oss;
        boost::property_tree::write_xml(oss, tree_);
        oss << msg_end;

        new_socket->send(boost::asio::buffer(oss.str()), 0);

        tree_.clear();

        if (resp != SERVER_RESP_CODES::OK)
        {
            std::cout << "Try again\n";
            handle_accept(new_socket);
        }

        online_clients_[id] = new_socket;

        std::cout << "CLIENT ADDED!\n";

        std::this_thread::sleep_for(std::chrono::seconds(1));

        check_msg_queue(id);

        start_read(id);
    }

    void check_msg_queue(int id)
    {
        if (!MessageQueue::get_queue().check_msg_queue(id)) 
        {
            std::cout << "MSG QUEUE IS EMPTY!\n";
            return;
        }

        std::cout << "READ MSG QUEUE!\n";
        while (true)
        {
            auto opt_data = MessageQueue::get_queue().get_next_msg(id);
            if (!opt_data) break;
            
            BUFFER data = *opt_data;
            std::string msg = data->data() + std::string(msg_end);
            online_clients_[id]->send(boost::asio::buffer(msg), 0);
        }
    }

    void start_read(int id)
    {
        auto socket = online_clients_[id];

        BUFFER data(new boost::array<char, max_msg_length>);
        memset(data->data(), 0, max_msg_length);

        socket->async_receive(boost::asio::buffer(*data), 
            [this, data, socket, id](const boost::system::error_code& ec, size_t)
            {
                if (!ec)
                {
                    process_incoming_msg(socket, data, id);
                }
                else if (ec == boost::asio::error::eof) // if the client logged out
                {
                    online_clients_.erase(id);
                }
                else
                {
                    std::cerr << ec.what() << '\n';
                }
            });
    }

    void process_incoming_msg(SOCKET socket, BUFFER data, int id)
    {
        std::cout << "MESSAGE RECEIVED FROM " << id << '\n';

        std::stringstream ss(data->data());
        std::cout << ss.str() << '\n';
        read_xml(ss, tree_);
        
        bool system = tree_.get<bool>(MSG_TAGS::system);
        int sender = tree_.get<int>(MSG_TAGS::sender);

        if (system)
        {
            process_system_msg(socket, sender);
        }
        else
        {
            process_user_msg(data);
        }
        
        tree_.clear();

        start_read(id);
    }

    void process_system_msg(SOCKET socket, int sender)
    {
        int cmd = tree_.get<int>(SYSTEM_MSG_DATA::cmd);

        switch(cmd)
        {
            case SYSTEM_MSG::FIND_CONTACT:
            {
                std::string name = tree_.get<std::string>(SYSTEM_MSG_DATA::contact);
                auto contacts = db_manager_.find_contact(name);

                tree_.clear();

                for(const auto& c : contacts)
                {
                    std::cout << c.serialize() << '\n';
                    tree_.add(SYSTEM_MSG_DATA::contact, c.serialize());
                }

                break;
            }
            default:
            {
                std::cout << "Unknown system msg!\n";
                break;
            }
        }

        
        tree_.put(SYSTEM_MSG_DATA::system, true);
        tree_.put(SYSTEM_MSG_DATA::cmd, cmd);
        
        std::stringstream ss;
        boost::property_tree::write_xml(ss, tree_);
        ss << msg_end;

        std::cout << ss.str() << '\n';

        socket->send(boost::asio::buffer(ss.str()), 0);

        std::cout << "SYSTEM MSG PROCESSED\n";
    }

    void process_user_msg(BUFFER data)
    {
        std::vector<int> receivers;
            
        for(const auto& child : tree_.get_child(MSG_TAGS::msg))
        {
            if (child.first == "receiver")
            {
                int val = child.second.get<int>("");
                receivers.emplace_back(val);
            }
        }

        for(int recv : receivers)
        {
            try
            {
                if (online_clients_.contains(recv))
                {
                    std::string msg = data->data() + std::string(msg_end);
                    online_clients_[recv]->send(boost::asio::buffer(msg), 0);
                }
                else
                {
                    MessageQueue::get_queue().add_msg(recv, data);
                }
            }
            catch(const std::exception& e)
            {
                continue;
            }
        }
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

    std::unordered_map<int, SOCKET> online_clients_;

    DBServerManager db_manager_;
    ptree tree_;
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
