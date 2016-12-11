#pragma once

#ifndef ROUTERCONTROLLER_H
#define ROUTERCONTROLLER_H


#include <memory>
#include <thread>
#include <functional>
#include <boost/asio.hpp>
#include "router.hpp"


namespace Orthrus {


class RouterController
{
private:
    std::shared_ptr<boost::asio::io_service> io_service 
        = std::make_shared<boost::asio::io_service>();

    using error_handler_t = std::function<void(std::exception&)>;
    error_handler_t error_handler = 0;

    Router router;
    std::unique_ptr<std::thread> routerThread;
   
    void run();


public:
	RouterController(std::string hostname, unsigned short local_port);
    RouterController(std::string hostname, unsigned short local_port, error_handler_t&&);

    void set_error_handler(error_handler_t&& eh);

	Router& get_router();

    void start();  
    void stop();
};


} // namespace Orthrus


#endif // ROUTERCONTROLLER_H
