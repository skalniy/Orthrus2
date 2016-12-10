#include "peer.h"

Peer::Peer(std::shared_ptr<boost::asio::ip::tcp::socket> sock_, QObject *parent) try :
    QObject(parent), sock(sock_),
    buf(std::make_shared<boost::asio::streambuf>()),
    ist(std::make_shared<std::iostream>(buf.get()))
{
    boost::asio::read_until(*sock, *buf, '\n');
    std::getline(*ist, nickname);
    boost::asio::read_until(*sock, *buf, '\n');
    std::getline(*ist, remote_port);
    ist->clear();
} catch (boost::system::system_error& e) {
    emit error(nickname, nickname+" "+e.what());
}

Peer::~Peer() { sock->cancel(); sock->close(); }

void Peer::write(const std::string& msg) try
{
    boost::asio::async_write(*sock, boost::asio::buffer(msg),
        boost::bind(&Peer::write_handler, this, _1, _2));
} catch (boost::system::system_error& e) {
    emit error(nickname, nickname+" "+e.what());
}

void Peer::read_handler(const boost::system::error_code& ec,
    std::size_t bytes_transferred) try
{
    if (ec.value() == boost::system::errc::bad_file_descriptor)
        emit closedPipe(get_remote_address());
    else if (ec) {
        std::cout << "r" <<ec <<":" << ec.message() << std::endl;
        emit error(nickname, nickname+" "+ec.message());
        emit closedPipe(get_remote_address());
        return;
    }

    std::string msg;
    std::getline(*ist, msg);
    boost::asio::async_read_until(*sock, *buf, '\n',
        boost::bind(&Peer::read_handler, this, _1, _2));

    emit msgReceived(nickname, msg);
    std::cout << msg << std::endl;
} catch (boost::system::system_error& e) {
    emit error(nickname, nickname+" "+e.what());
}

void Peer::write_handler(const boost::system::error_code& ec,
    std::size_t bytes_transferred) try
{
    if (ec.value() == boost::system::errc::bad_file_descriptor)
        emit closedPipe(get_remote_address());
    else if (ec) {
        std::cout << "w" <<ec <<":" << ec.message() << std::endl;
        emit error(nickname, nickname+" "+ec.message());
        return;
    }

} catch (boost::system::system_error& e) {
    emit error(nickname, nickname+" "+e.what());
}
