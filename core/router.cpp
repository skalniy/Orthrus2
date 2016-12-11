#include "router.hpp"
#include <iostream>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <string>



using namespace Orthrus;


Router::Router(std::shared_ptr<boost::asio::io_service> io_service,
        std::string hostname_, unsigned short local_port) 
    : hostname(hostname_)
    , ep(boost::asio::ip::tcp::v4(), local_port)
    , acceptor(std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service, ep, true))  
{}


Router::Router(std::shared_ptr<boost::asio::io_service> io_service,
        std::string hostname_, unsigned short local_port, error_handler_t& eh) 
    : Router(io_service, hostname_, local_port) 
{  error_handler = eh; }


Router::~Router() 
{ stop(); }


void Router::start() try
{
    acceptor->listen();
    init();
}
catch (std::exception& e) { error_handler(e); }


void Router::stop() try
{
    if (!acceptor->is_open())
        return;
    acceptor->cancel();
    acceptor->close();
}
catch (std::exception& e) { error_handler(e); }


void Router::init() try
{
    std::shared_ptr<boost::asio::ip::tcp::socket> sock 
        = std::make_shared<boost::asio::ip::tcp::socket>(
            acceptor->get_io_service());
    acceptor->async_accept(*sock, 
        boost::bind(&Router::accept_handler, this, _1, sock));
}
catch (std::exception& e) { error_handler(e); }


void Router::set_read_msg_cb(read_msg_cb_t&& cb) try
{ 
    read_msg_cb = cb;
    for (auto peer : peers)
        peer.second->read_msg_cb = cb;
}
catch (std::exception& e) { error_handler(e); }


void Router::set_error_handler(error_handler_t& eh) try
{ 
    error_handler = eh;
    for (auto peer : peers)
        peer.second->error_handler = error_handler;
}
catch (std::exception& e) { error_handler(e); }

void Router::accept_handler(const boost::system::error_code& error,
    std::shared_ptr<boost::asio::ip::tcp::socket> sock) try
{
    if (error) {
        throw std::runtime_error(error.message());
    }
    init();

    boost::asio::write(*sock, boost::asio::buffer(hostname + '\n'));
    boost::asio::write(*sock, boost::asio::buffer(std::to_string(acceptor->local_endpoint().port()) + '\n'));
    std::cout <<"sended" << acceptor->local_endpoint()<<std::endl;
    std::shared_ptr<Peer> new_peer = make_peer(sock);
    new_peer->listen();
    share_peers(new_peer->get_sock());
    peers.emplace(new_peer->get_remote_address(), new_peer);
}
catch (std::exception& e) { error_handler(e); }


std::shared_ptr<Peer> Router::make_peer(
    std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    std::shared_ptr<Peer> new_peer = std::make_shared<Peer>(sock, error_handler);
    new_peer->read_msg_cb = read_msg_cb;
    new_peer->disconnect_handler 
        = boost::bind(&Router::disconnect, this, new_peer->get_remote_address());

    return new_peer;
}


void Router::send_msg(std::string msg) try
{
    for (auto peer_ : peers) 
        peer_.second->write(msg+'\n'); 
}
catch (std::exception& e) { error_handler(e); }


void Router::connect(std::string addr) try
{
    std::vector<std::string> ep_params;
    ep_params = 
        boost::algorithm::split(ep_params, addr, boost::algorithm::is_any_of(":"));
    boost::asio::ip::tcp::endpoint remote_ep = 
        *boost::asio::ip::tcp::resolver(acceptor->get_io_service()).resolve(
                boost::asio::ip::tcp::resolver::query(
                    boost::asio::ip::tcp::v4(), ep_params[0], ep_params[1]
                )
            );
    
    std::shared_ptr<boost::asio::ip::tcp::socket> sock = 
        std::make_shared<boost::asio::ip::tcp::socket>(acceptor->get_io_service());
    std::cout << "ing "<<remote_ep<<std::endl;
    sock->connect(remote_ep);
    std::cout << "ed "<<remote_ep<<std::endl;

    std::shared_ptr<Peer> peer = make_peer(sock);
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(hostname + '\n'));
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(std::to_string(ep.port()) + '\n'));

    peers.emplace(peer->get_remote_address(), peer);
    read_peers(peer->get_sock());
    peer->listen();
}
catch (std::exception& e) { error_handler(e); }


void Router::share_peers(std::shared_ptr<boost::asio::ip::tcp::socket> target) try
{
    boost::asio::write(*target, boost::asio::buffer(std::to_string(peers.size()) + '\n'));
    for (auto peer : peers)
        boost::asio::write(*target, 
            boost::asio::buffer(peer.second->get_remote_address() + '\n')
        );
}
catch (std::exception& e) { error_handler(e); }


void Router::read_peers(std::shared_ptr<boost::asio::ip::tcp::socket> sock) try
{
    boost::asio::streambuf buf;
    std::istream ist(&buf);

    boost::asio::read_until(*sock, buf, '\n');
    size_t peers_amount;
    ist >> peers_amount;
    for (size_t i = 0; i < peers_amount; i++) {
        std::string misc, addr;

        boost::asio::read_until(*sock, buf, '\n');
        std::getline(ist, misc);
        boost::asio::read_until(*sock, buf, '\n');
        std::getline(ist, addr);

        if (peers.find(addr) == peers.end()) 
            connect(addr);
    }
}
catch (std::exception& e) { error_handler(e); }
