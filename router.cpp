#include "router.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

Router::Router(
        QObject *parent,
        std::shared_ptr<boost::asio::io_service> io_service,
        unsigned short port_num,
        std::string hostname_
       )
    : QObject(parent)
    , hostname(hostname_)
    , ep(boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port_num))
    , acceptor(std::make_shared<boost::asio::ip::tcp::acceptor>(*io_service, ep, true))
{
    acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
}

void Router::start()
{
    acceptor->listen();
    init();
}

void Router::stop()
{
    if (acceptor->is_open()) {
        acceptor->cancel();
        acceptor->close();
    }
}

void Router::init()
{
    std::shared_ptr<boost::asio::ip::tcp::socket> sock =
        std::make_shared<boost::asio::ip::tcp::socket>(
            acceptor->get_io_service()
        );
    acceptor->async_accept(*sock,
        boost::bind(&Router::accept_handler, this, _1, sock));
}

void Router::share_peers(std::shared_ptr<boost::asio::ip::tcp::socket> target)
{
    boost::asio::write(*target, boost::asio::buffer(std::to_string(peers.size()) + '\n'));
    for (auto peer : peers)
        boost::asio::write(*target,
            boost::asio::buffer(peer.second->get_remote_address() + '\n'));
}

void Router::accept_handler(const boost::system::error_code& error,
    std::shared_ptr<boost::asio::ip::tcp::socket> sock)
{
    std::cout << "accepted" << std::endl;
    if (error) {
        std::cout << "accept error: " << error.message() << std::endl;
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
    QObject::connect(new_peer.get(), &Peer::msgReceived, this, &Router::msgReceived);
    new_peer->listen();
    share_peers(new_peer->get_sock());
    // либо красными буквами писать, что io_service запускать только в одном треде,
    // либо таки синхронизацию добавить
    // с io_service в одном треде один медленный абонент тормозит всех
    // может, все операции сделать асинхронными?
    peers.emplace(new_peer->get_remote_address(), new_peer);
}

void Router::msgSendSlot(std::string msg) {
    for (auto peer : peers)
        peer.second->write(msg+'\n'); // что с многострочными сообщениями?
}

void Router::connect(std::string addr)
{
    std::cout << "finding " << addr << std::endl;
    std::vector<std::string> ep_params;
    ep_params =
        boost::algorithm::split(ep_params, addr, boost::algorithm::is_any_of(":"));
    boost::asio::ip::tcp::endpoint remote_ep =
        *boost::asio::ip::tcp::resolver(acceptor->get_io_service()).resolve(
            boost::asio::ip::tcp::resolver::query(
                boost::asio::ip::tcp::v4(), ep_params[0], ep_params[1]
            )
        );
 //   boost::asio::ip::tcp::endpoint remote_ep(boost::asio::ip::address_v4::from_string(ip), port);
    std::shared_ptr<boost::asio::ip::tcp::socket> sock =
        std::make_shared<boost::asio::ip::tcp::socket>(acceptor->get_io_service());
    sock->connect(remote_ep);
    std::cout << "connected to " << remote_ep << std::endl;

    std::shared_ptr<Peer> peer = std::make_shared<Peer>(sock);
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(hostname + '\n'));
    boost::asio::write(*peer->get_sock(), boost::asio::buffer(std::to_string(ep.port()) + '\n'));

    peers.emplace(peer->get_remote_address(), peer);
    read_peers(peer->get_sock());
    QObject::connect(peer.get(), &Peer::msgReceived, this, &Router::msgReceived);
    peer->listen();
}


void Router::read_peers(std::shared_ptr<boost::asio::ip::tcp::socket> sock)
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
}
