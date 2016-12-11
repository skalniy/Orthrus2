#pragma once

#ifndef ROUTER_H
#define ROUTER_H


#include <memory>
#include <thread>
#include <map>
#include <boost/asio.hpp>
#include "peer.hpp"



namespace Orthrus {


class Router
{
private:
    std::string hostname;

    using read_msg_cb_t = std::function<void(std::string, std::string)>;
    read_msg_cb_t read_msg_cb = 0;

    boost::asio::ip::tcp::endpoint ep;

    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;

    using error_handler_t = std::function<void(std::exception&)>;
    error_handler_t error_handler = 0;

    std::map<std::string, std::shared_ptr<Peer>> peers;

    void init();

    void accept_handler(const boost::system::error_code& error,
        std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    void share_peers(std::shared_ptr<boost::asio::ip::tcp::socket> target);

    void read_peers(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    std::shared_ptr<Peer> make_peer(std::shared_ptr<boost::asio::ip::tcp::socket>);

    
public:
    using send_t = std::function<void(std::string)>;
    const send_t send
        = std::bind(&Router::send_msg, this, std::placeholders::_1);

    using accept_notifier_t = std::function<void(std::string)>;
    accept_notifier_t accept_notifier = 0;

    Router(std::shared_ptr<boost::asio::io_service> io_service, 
        std::string hostname, unsigned short local_port);
    Router(std::shared_ptr<boost::asio::io_service> io_service, 
        std::string hostname, unsigned short local_port, error_handler_t&);
    ~Router();

    void set_read_msg_cb(read_msg_cb_t&& cb);

    void disconnect(std::string addr) {
        peers.erase(addr);
    }
    
    void start();

    void stop();

    void connect(std::string);

    void send_msg(std::string msg);

    void set_error_handler(error_handler_t& eh);
};


}  // namespace Orthrus



#endif // ROUTER_H
