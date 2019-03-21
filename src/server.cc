#include "server.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <thread>

void server::start_accept()
{
    //Multithreading
    session::pointer new_session = session::create(io_service_, config_);

    //Call to start connection. New session per request
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, this, new_session,
          boost::asio::placeholders::error));
    
}

void server::handle_accept(session::pointer new_session,
    const boost::system::error_code& error)
{
    if (!error)
    {
        BOOST_LOG_TRIVIAL(trace) << "Starting new session...";
        new_session->start();
    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "Error! Session did not start...";
        //delete new_session;
    }
    start_accept();
}

void server::handle_stop()
{
  io_service_.stop();
}