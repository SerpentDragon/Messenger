#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <iterator>
#include <mutex>
#include <pqxx/pqxx>
#include <sstream>
#include <unordered_map>

#include "DB_Manager/database.h"
#include "ServerUtils/cryptographer.h"
#include "ServerUtils/messagequeue.h"
#include "ServerUtils/timer.h"
#include "../API/resp_codes.h"
#include "../API/protocols.h"
#include "../API/common_structure.h"

using boost::asio::ip::tcp;
using boost::property_tree::ptree;

class Server : public std::enable_shared_from_this<Server>
{
private:

    using BUFFER = boost::shared_ptr<boost::array<char, max_msg_length>>;
    using SOCKET = boost::shared_ptr<tcp::socket>;

public:

    Server(boost::asio::io_service& io, const int port) :
        io_(io), acceptor_(io_, tcp::endpoint(tcp::v4(), port))
    {
        db_manager_.connect("dbname=messenger_server user=dmitry password=1223684DS hostaddr=127.0.0.1 port=5432");

        Cryptographer::get_cryptographer()->generate_RSA_keys();
        std::tie(private_key_, public_key_) = Cryptographer::get_cryptographer()->serialize_RSA_keys();

        std::cout << "READY TO LOAD KEYS TO DB";

        db_manager_.save_RSA_keys({ private_key_, public_key_ });

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
                if (!ec) 
                {
                    auto client_public_key = read_keys(new_socket);
                    handle_accept(new_socket, client_public_key);
                }
                else std::cerr << "Error accepting new connection: " << ec.message() << '\n';
            });
    }

    std::string read_keys(SOCKET new_socket)
    {
        BUFFER data(new boost::array<char, max_msg_length>);
        memset(data->data(), 0, max_msg_length);

        new_socket->send(boost::asio::buffer(public_key_));

        // std::cout << "SEND SERVER PUBLIC KEY:\n" << public_key_ << "\n\n";

        new_socket->receive(boost::asio::buffer(*data));

        std::string client_public_key = data->data();

        return client_public_key;

        // std::cout << "RESEIVED CLIENT'S PUBLIC KEY:\n" << client_public_key << "\n\n";
    }

    void handle_accept(SOCKET new_socket, const std::string public_key)
    {
        std::cout << __func__ << '\n';

        BUFFER data(new boost::array<char, max_msg_length>);
        memset(data->data(), 0, max_msg_length);
        
        new_socket->async_receive(boost::asio::buffer(*data),
            [this, data, new_socket, public_key](const boost::system::error_code& ec, size_t len)
            {
                std::cout << data->data() << '\n';
                if (!ec) 
                {
                    std::vector<uint8_t> msg(data->begin(), data->begin() + len);
                    if (msg[0] == 0) msg.erase(msg.begin());

                    for(int c : msg) std::cout << std::dec << (int)c << ' ';
                    std::cout << '\n';

                    auto result = Cryptographer::get_cryptographer()->decrypt_AES(msg);

                    std::cout << "LOG IN/SIGN UP: " << result << '\n';

                    add_client(new_socket, public_key, result);
                }
                else std::cerr << "Error receiving data: " << ec.message() << '\n';
            });

        start_accept();
    }

    void add_client(SOCKET new_socket, const std::string& cl_pub_key, const std::string& data)
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
            std::cout << "psswd_frm_cl: " << password << '\n';
            std::cout << (password_from_db == password) << '\n';

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

        std::cout << "answer to client: " << oss.str() << '\n';

        auto encrypted = Cryptographer::get_cryptographer()->encrypt_AES(oss.str(), cl_pub_key);
        encrypted.insert(encrypted.end(), std::begin(msg_end), std::end(msg_end) - 1);

        new_socket->send(boost::asio::buffer(encrypted));

        if (resp != SERVER_RESP_CODES::OK)
        {
            std::cout << "Try again\n";
            handle_accept(new_socket, cl_pub_key);
        }

        // online_clients_[id] = Client{ .socket = new_socket, .mtx = {}, .public_key = cl_pub_key };

        auto res = online_clients_.try_emplace(id, new_socket, cl_pub_key);

        std::cout << "CLIENT ADDED! " << res.second << '\n';

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

            std::vector<uint8_t> data;
            if (opt_data->first)
            {
                std::string msg(opt_data->second.begin(), opt_data->second.end());
                std::cout << "DATA TO BE ENCRYPTED:\n" << msg << "\n-------------------------------------------\n";
                data = Cryptographer::get_cryptographer()->encrypt_AES(msg, online_clients_[id].public_key);
                data.insert(data.end(), std::begin(msg_end), std::end(msg_end) - 1);
            }
            else
            {
                data = opt_data->second;
            }

            online_clients_[id].socket->send(boost::asio::buffer(data));
        }
    }

    void start_read(int id)
    {
        auto socket = online_clients_.at(id).socket;

        BUFFER data_ptr(new boost::array<char, max_msg_length>);
        memset(data_ptr->data(), 0, max_msg_length);

        boost::asio::async_read_until(*socket, stream_buf_, msg_end,
        [this, data_ptr, socket, id](const boost::system::error_code& ec, std::size_t bytes_transferred)
            {
                if (!ec)
                {
                    std::size_t len = bytes_transferred < max_msg_length ?
                                    bytes_transferred : max_msg_length;

                    std::istream is(&stream_buf_);
                    is.read(data_ptr->data(), len - strlen(msg_end));
                    stream_buf_.consume(strlen(msg_end));

                    // std::cout << "RECEIVED MSG:\n";
                    // for(int c : data_ptr->) std::cout << c << ' ';
                    // std::cout << "\n-----------------------------------------------------------------------------\n";

                    std::vector<uint8_t> data2(data_ptr->begin(), 
                        data_ptr->begin() + len);

                    std::vector<uint8_t> data(data_ptr->begin(), 
                        data_ptr->begin() + len - strlen(msg_end));
                    if (data[0] == 0) data.erase(data.begin());

                    std::cout << "RECEIVED MSG:\n";
                    for(int c : data) std::cout << c << ' ';
                    std::cout << "\n-----------------------------------------------------------------------------\n";

                    if (std::string msg_str(data.begin(), data.begin() + len - strlen(msg_end));
                        msg_str.find(recv_open_tag) != std::string::npos)
                    {
                        // std::cout << "USER MESSAGE: " << msg_str << '\n';
                        // std::cout << "RECEIVED USER MSG:\n";
                        // for(int c : msg_str) std::cout << c << ' ';
                        // std::cout << "\n-----------------------------------------------------------------------------\n";
                        process_user_msg(msg_str);
                    }
                    else
                    {
                        std::vector<uint8_t> msg_data(data.begin(), data.begin() + len - strlen(msg_end));
                        if (msg_data[0] == 0) msg_data.erase(msg_data.begin());

                        auto result = Cryptographer::get_cryptographer()->decrypt_AES(msg_data);

                        std::cout << "START_READ: " << result << '\n';

                        process_system_msg(result);
                    }

                    start_read(id);
                }
                else if (ec == boost::asio::error::eof) // if the client logged out
                {
                    online_clients_.erase(id);
                }
                else
                {
                    std::cerr << ec.what() << '\n';
                }
                
            }
        );
    }

    void process_system_msg(const std::string& decrypted_data)
    {
        tree_.clear();

        std::vector<int> recvrs;

        std::stringstream ss(decrypted_data);
        std::cout << "\"" << ss.str() << "\"\n";
        read_xml(ss, tree_);

        int cmd = tree_.get<int>(SYSTEM_MSG_DATA::cmd);
        int sender = tree_.get<int>(MSG_TAGS::sender);

        std::cout << "MESSAGE RECEIVED FROM " << sender << '\n';
        
        switch(cmd)
        {
            case SYSTEM_MSG::LOAD_RSA_KEY:
            {
                std::string key = tree_.get<std::string>(SYSTEM_MSG_DATA::key);

                tree_.clear();

                db_manager_.load_RSA_key(sender, key);
                online_clients_.at(sender).public_key = key;

                check_msg_queue(sender);

                break;
            }
            case SYSTEM_MSG::FIND_CONTACT:
            {
                std::string name = tree_.get<std::string>(SYSTEM_MSG_DATA::contact);

                tree_.clear();

                std::cout << "NAME TO FIND: " << name << '\n';

                auto contacts = db_manager_.find_contact(sender, name);

                for(const auto& c : contacts)
                {
                    std::cout << c.serialize() << '\n';
                    tree_.add(SYSTEM_MSG_DATA::contact, c.serialize());
                }

                break;
            }
            case SYSTEM_MSG::GET_CONTACT:
            {
                int id = tree_.get<int>(SYSTEM_MSG_DATA::contact);

                tree_.clear();

                auto contact = db_manager_.get_contact(id);

                tree_.put(SYSTEM_MSG_DATA::contact, contact.serialize());

                break;
            }
            case SYSTEM_MSG::NEW_GROUP_CHAT:
            {
                int chat_time;
                std::string name;
                std::vector<int> members;

                for(const auto& tag : tree_.get_child(SYSTEM_MSG_DATA::data))
                {
                    if (tag.first == "chat_time")
                    {
                        chat_time = tree_.get<int>(SYSTEM_MSG_DATA::chat_time);
                    }
                    else if (tag.first == "chat_name")
                    {
                        name = tree_.get<std::string>(SYSTEM_MSG_DATA::chat_name);
                    }
                    if (tag.first == "chat_member")
                    {
                        members.emplace_back(std::stoi(tag.second.data()));
                    }
                }

                tree_.clear();

                int chat_id = db_manager_.save_chat(name, members);

                if (chat_time != -1)
                {
                    std::time_t now_time = std::time(nullptr);
                    int diff_seconds = static_cast<int>(std::difftime(chat_time, now_time));

                    std::cout << "Prepare to start a timer for chat " << chat_id << '\n';
                    Timer timer;
                    timer.start( diff_seconds, [this, members, chat_id]()
                        {
                            db_manager_.delete_chat(chat_id);
                        });
                }

                tree_.put(SYSTEM_MSG_DATA::chat_id, chat_id);
                tree_.put(SYSTEM_MSG_DATA::chat_name, name);
                tree_.put(SYSTEM_MSG_DATA::chat_time, chat_time);

                recvrs = members;

                for(int mem : members)
                {
                    auto contact = db_manager_.get_contact(mem);
                    tree_.add(SYSTEM_MSG_DATA::contact, contact.serialize());
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
        
        ss = std::stringstream();
        boost::property_tree::write_xml(ss, tree_);

        std::vector<uint8_t> encrypted;

        std::cout << "PREPARE TO SEND---------------------------------\n" << ss.str() << 
            "--------------------------------------------------" << '\n';

        if (cmd == SYSTEM_MSG::NEW_GROUP_CHAT)
        {
            for(int recv : recvrs)
            {
                send_data(ss.str(), recv);
            }

            return;
        }

        send_data(ss.str(), sender);

        std::cout << "SYSTEM MSG PROCESSED\n";
    }

    void process_user_msg(const std::string& msg)
    {
        std::cout << "USER MSG\n";

        auto end = msg.find(recv_close_tag);
        std::string num = msg.substr(strlen(recv_open_tag), end - strlen(recv_open_tag));
        int receiver = std::stoi(num);

        std::cout << "SEND TO:" << receiver << '\n';

        std::vector<uint8_t> client_data(msg.begin() + end + strlen(recv_close_tag), msg.end());
        client_data.insert(client_data.end(), std::begin(msg_end), std::end(msg_end) - 1);

        std::cout << "SEND IT TO USER: " << client_data.size() << "\n";
        for(int c : client_data) std::cout << c << ' ';
        std::cout << "\n-----------------------------------------------------------------------------\n";


        if (online_clients_.contains(receiver))
        {
            online_clients_.at(receiver).socket->send(boost::asio::buffer(client_data));
        }
        else
        {
            MessageQueue::get_queue().add_msg(receiver, false, client_data);
        }
    }
    
private:

    static void build_response(ptree& tree, SERVER_RESP_CODES rc, int id)
    {
        tree.put(SERVER_RESPONSE::status, rc);
        tree.put(SERVER_RESPONSE::id, id);
    }

    void send_data(const std::string& data, int recv)
    {
        std::cout << "SEND MESSAGE TO: " << recv << '\n';

        std::vector<uint8_t> encrypted;

        if (online_clients_.contains(recv))
        {
            encrypted = Cryptographer::get_cryptographer()->encrypt_AES(data, online_clients_.at(recv).public_key);
            encrypted.insert(encrypted.end(), std::begin(msg_end), std::end(msg_end) - 1);

            std::lock_guard<std::mutex> lock_g(online_clients_.at(recv).mtx);  
            online_clients_.at(recv).socket->send(boost::asio::buffer(encrypted));
            std::cout << "MESSAGE WAS SENT TO " << recv << "\n";
        }
        else
        {
            encrypted.insert(encrypted.end(), data.begin(), data.end());
            MessageQueue::get_queue().add_msg(recv, true, encrypted);
        }
    }

private:

    struct Client
    {
        SOCKET socket;
        std::mutex mtx;
        std::string public_key;

        Client() = default;

        explicit Client(SOCKET sock, const std::string& key)
            : socket(sock), public_key(key) {};
    };

    boost::asio::io_service& io_;
    tcp::acceptor acceptor_;

    boost::asio::streambuf stream_buf_;

    std::unordered_map<int, Client> online_clients_;

    DBServerManager db_manager_;
    ptree tree_;

    std::string private_key_;
    std::string public_key_;
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
