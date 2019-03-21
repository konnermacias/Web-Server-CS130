#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include "session.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

class server
{
  public:
    server(boost::asio::io_service& io_service, short port, NginxConfig* config)
        : io_service_(io_service),
          signals_(io_service),
          acceptor_(io_service, tcp::endpoint(tcp::v4(), port))
    {
        config_ = config;
        start_accept();
    }
    //Used to testing purposes
    friend class Serv;

  private:
    void start_accept();
    void handle_accept(session::pointer new_session, const boost::system::error_code& error);
    void handle_stop();
    
    std::size_t thread_pool_size_;
    boost::asio::io_service& io_service_;
    boost::asio::signal_set signals_;
    tcp::acceptor acceptor_;
    NginxConfig* config_;
};

class Serv {
  public:
    void call_start_accept(server* serv) {
      serv->start_accept();
    }
    void call_handle_accept(server* serv, session::pointer new_session, const boost::system::error_code& error) {
      serv->handle_accept(new_session, error);
    }
    void call_handle_stop(server* serv) {
      serv->handle_stop();
    }
};