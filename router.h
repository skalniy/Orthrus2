#ifndef ROUTER_H
#define ROUTER_H

#include <QObject>
#include <QMessageBox>
#include <memory>
#include <string>
#include <boost/asio.hpp>
#include "peer.h"

class Router : public QObject
{
    Q_OBJECT // qt везде пробрался, причем, мне кажется, необоснованно
    // хотите взаимодействовать сигналами слотами -- делайте адаптор,
    // но не портить же все классы зависимостью от qt!

    std::string hostname;
    boost::asio::ip::tcp::endpoint ep;
    std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    std::map<std::string, std::shared_ptr<Peer>> peers;

    void init();

    void accept_handler(const boost::system::error_code& error,
        std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    void share_peers(std::shared_ptr<boost::asio::ip::tcp::socket> target);
    void read_peers(std::shared_ptr<boost::asio::ip::tcp::socket> sock);

    friend class RouterController;

public:
    explicit Router(QObject *parent, std::shared_ptr<boost::asio::io_service> io_service,
                    unsigned short port_num, std::string hostname);

    void start();
    void stop();

signals:
    void msgReceived(std::string, std::string); // это решается через callback

public slots:
    void msgSendSlot(std::string msg); // это решается через методы
    void connect(std::string addr);

};

#endif // ROUTER_H
