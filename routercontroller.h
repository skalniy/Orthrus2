#ifndef ROUTERCONTROLLER_H
#define ROUTERCONTROLLER_H

#include <QObject>
#include <memory>
#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#include "router.h"

class RouterController : public QObject
{
    Q_OBJECT

    std::shared_ptr<boost::asio::io_service> io_service;
    Router router;
    std::unique_ptr<std::thread> routerThread;

    void run();

public:
    explicit RouterController(QObject *parent,
                              unsigned short port_num,
                              std::string hostname);
    ~RouterController();

    inline std::string get_hostname() { return router.hostname; }

signals:
    void msgReceived(std::string, std::string);
    void msgSend(std::string);
    void connectClicked(std::string addr);

public slots:
    void start(void);
    void stop(void);

};

#endif // ROUTERCONTROLLER_H
