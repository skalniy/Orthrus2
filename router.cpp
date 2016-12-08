#include "router.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

Router::Router(
        QObject *parent,
        std::shared_ptr<boost::asio::io_service> io_service,
        unsigned short port_num,
        std::string hostname_
       ) try
    : QObject(parent)
    , hostname(hostname_)
    , ep(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num))
    , acceptor(std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service, ep, true))
{} catch (boost::system::system_error& e) {
   emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::start() try
{
    acceptor->listen();
    init();
} catch (boost::system::system_error& e) {
   emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::stop() try
{
    if (acceptor->is_open()) {
        acceptor->cancel();
        acceptor->close();
    }
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::init() try
{
    std::shared_ptr<boost::asio::ip::tcp::socket> sock =
        std::make_shared<boost::asio::ip::tcp::socket>(
            acceptor->get_io_service()
        );
    acceptor->async_accept(*sock,
        boost::bind(&Router::accept_handler, this, _1, sock));
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::share_peers(std::shared_ptr<boost::asio::ip::tcp::socket> target) try
{
    boost::asio::write(*target, boost::asio::buffer(std::to_string(peers.size()) + '\n'));
    for (auto peer : peers)
        boost::asio::write(*target,
            boost::asio::buffer(peer.second->get_remote_address() + '\n'));
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::accept_handler(const boost::system::error_code& error_,
    std::shared_ptr<boost::asio::ip::tcp::socket> sock) try
{
    if (error_) {
        emit error("Acceptor", "acc "+error_.message());
        return;
    }
    init();

    boost::asio::write(*sock, boost::asio::buffer(hostname + '\n'));
    boost::asio::write(
                *sock,
                boost::asio::buffer(
                    std::to_string(acceptor->local_endpoint().port()) + '\n'
                    )
                );
    std::shared_ptr<Peer> new_peer = std::make_shared<Peer>(sock);
    QObject::connect(new_peer.get(), &Peer::error, this, &Router::error);
    QObject::connect(new_peer.get(), &Peer::closedPipe, this, &Router::closedPipe);
    QObject::connect(new_peer.get(), &Peer::msgReceived, this, &Router::msgReceived);
    new_peer->listen();
    share_peers(new_peer->get_sock());
    peers.emplace(new_peer->get_remote_address(), new_peer);
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::msgSendSlot(std::string msg)  try {
    for (auto peer : peers)
        peer.second->write(msg+'\n');
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}

void Router::connect(std::string addr) try
{
    std::vector<std::string> ep_params;
    boost::algorithm::split(ep_params, addr, boost::algorithm::is_any_of(":"));

    boost::asio::ip::tcp::endpoint remote_ep =
        *boost::asio::ip::tcp::resolver(acceptor->get_io_service()).resolve(
            boost::asio::ip::tcp::resolver::query(
                boost::asio::ip::tcp::v4(), ep_params[0], ep_params[1]
            )
        );

    std::shared_ptr<boost::asio::ip::tcp::socket> sock =
        std::make_shared<boost::asio::ip::tcp::socket>(acceptor->get_io_service());
    sock->connect(remote_ep);

    std::shared_ptr<Peer> peer = std::make_shared<Peer>(sock);
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(hostname + '\n'));
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(std::to_string(ep.port()) + '\n'));

    peers.emplace(peer->get_remote_address(), peer);
    read_peers(peer->get_sock());
    QObject::connect(peer.get(), &Peer::error, this, &Router::error);
    QObject::connect(peer.get(), &Peer::closedPipe, this, &Router::closedPipe);
    QObject::connect(peer.get(), &Peer::msgReceived, this, &Router::msgReceived);
    peer->listen();
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}


void Router::read_peers(std::shared_ptr<boost::asio::ip::tcp::socket> sock) try
{
    boost::asio::streambuf buf;
    std::istream ist(&buf);

    boost::asio::read_until(*sock, buf, '\n');
    size_t peers_amount;
    ist >> peers_amount;
    for (auto i = 0; i < peers_amount; i++) {
        std::string misc, addr;

        boost::asio::read_until(*sock, buf, '\n');
        std::getline(ist, misc);
        boost::asio::read_until(*sock, buf, '\n');
        std::getline(ist, addr);

        if (peers.find(addr) == peers.end())
            connect(addr);
    }
} catch (boost::system::system_error& e) {
    emit error("Acceptor", "acc "+std::string(e.what()));
}


void Router::closedPipe(std::string addr) {
    emit msgReceived(peers[addr]->get_nickname(), "DISCONNECTED");
    peers.erase(addr);
}
