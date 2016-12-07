#include "routercontroller.h"

RouterController::RouterController(QObject *parent,
                                   unsigned short port_num,
                                   std::string hostname) try
    : QObject(parent)
    , io_service(std::make_shared<boost::asio::io_service>())
    , router(this, io_service, port_num, hostname)
    , routerThread(nullptr)
{
    QObject::connect(&router, &Router::error, this, &RouterController::error);
    QObject::connect(&router, &Router::msgReceived, this, &RouterController::msgReceived);
    QObject::connect(this, &RouterController::msgSend, &router, &Router::msgSendSlot);
    QObject::connect(this, &RouterController::connectClicked, &router, &Router::connect);
} catch (boost::system::system_error& e) {
    emit error("Router", e.what());
}

RouterController::~RouterController() try
{
    stop();
} catch (boost::system::system_error& e) {
    emit error("Router", e.what());
}

void RouterController::start(void) try
{
    routerThread.reset(new std::thread(&RouterController::run, this));
} catch (boost::system::system_error& e) {
    emit error("Router", e.what());
}

void RouterController::stop() try
{
    io_service->stop();
    router.stop();
    if (routerThread.get())
        routerThread->join();
} catch (boost::system::system_error& e) {
    emit error("Router", e.what());
}

void RouterController::run() try
{
    router.start();
    io_service->run();
} catch (boost::system::system_error& e) {
    emit error("Router", e.what());
}
