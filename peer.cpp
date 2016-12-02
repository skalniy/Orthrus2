#include "peer.h"

Peer::Peer(std::shared_ptr<boost::asio::ip::tcp::socket> sock_, QObject *parent) :
    QObject(parent), sock(sock_),
    buf(std::make_shared<boost::asio::streambuf>()),
    ist(std::make_shared<std::iostream>(buf.get()))
{
    boost::asio::read_until(*sock, *buf, '\n');
    std::getline(*ist, nickname);
    boost::asio::read_until(*sock, *buf, '\n');
    std::getline(*ist, remote_port);
    ist->clear();
    std::cout <<  nickname << remote_port;
}

void Peer::write(const std::string& msg)
{
    boost::asio::async_write(*sock, boost::asio::buffer(msg),
        boost::bind(&Peer::write_handler, this, _1, _2));
}

void Peer::read_handler(const boost::system::error_code& ec,
    std::size_t bytes_transferred)
{
    if (ec) {
        std::cout << "error: " << ec.message() << std::endl;
        return;
    }

    std::string msg;
    std::getline(*ist, msg);
    boost::asio::async_read_until(*sock, *buf, '\n',
        boost::bind(&Peer::read_handler, this, _1, _2));

    emit msgReceived(nickname, msg);
    std::cout << msg << std::endl;
}

void Peer::write_handler(const boost::system::error_code& error,
    std::size_t bytes_transferred)
{
    if (error) {
        std::cout << "error: " << error.message() << std::endl;
        return;
    }
}
