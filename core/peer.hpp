#pragma once

#ifndef PEER_H
#define PEER_H


#include <memory>
#include <iostream>
#include <functional>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>



namespace Orthrus {


class Peer
{
private:
    std::string nickname;

    std::string remote_port;

    std::shared_ptr<boost::asio::ip::tcp::socket> sock; 

    std::shared_ptr<boost::asio::streambuf> buf =
        std::make_shared<boost::asio::streambuf>();

    std::shared_ptr<std::istream> ist = 
        std::make_shared<std::iostream>(buf.get());


public:
    using read_msg_cb_t = std::function<void(std::string, std::string)>;
    read_msg_cb_t read_msg_cb = 0;

    using error_handler_t = std::function<void(std::exception&)>;
    error_handler_t error_handler = 0;

    using connection_handler_t  = std::function<void()>;
    connection_handler_t disconnect_handler;
    connection_handler_t connect_handler;

    Peer(std::shared_ptr<boost::asio::ip::tcp::socket>);
    Peer(std::shared_ptr<boost::asio::ip::tcp::socket>, error_handler_t&);
    ~Peer();

    std::string get_nickname();    
    std::string get_remote_address();

    std::shared_ptr<boost::asio::ip::tcp::socket> get_sock();

    void listen();
    void write(const std::string&);

    void read_handler(const boost::system::error_code&, std::size_t);
    void write_handler(const boost::system::error_code&, std::size_t);
};


}  // namespace Orthrus



#endif // PEER_H