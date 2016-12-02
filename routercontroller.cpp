#include "routercontroller.h"

RouterController::RouterController(QObject *parent,
                                   unsigned short port_num,
                                   std::string hostname)
    : QObject(parent)
    , io_service(std::make_shared<boost::asio::io_service>())
    , router(this, io_service, port_num, hostname)
    , routerThread(nullptr)
{
    QObject::connect(&router, &Router::msgReceived, this, &RouterController::msgReceived);
    QObject::connect(this, &RouterController::msgSend, &router, &Router::msgSendSlot);
    QObject::connect(this, &RouterController::connectClicked, &router, &Router::connect);
}

RouterController::~RouterController()
{
    stop();
}

void RouterController::start(void)
{
    routerThread.reset(new std::thread(&RouterController::run, this));
}

void RouterController::stop()
{
    io_service->stop();
    router.stop();
    if (routerThread.get())
        routerThread->join();
}

void RouterController::run()
{
    router.start();
    io_service->run();
}
