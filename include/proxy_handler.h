#pragma once

#include <boost/asio.hpp>
#include <map>
#include <string>
#include <vector>
#include "request.h"
#include "response.h"
#include "request_handler.h"
#include "config_parser.h"
#include <regex>
#include <iterator>

using boost::asio::ip::tcp;
using namespace std;

class SocketInf
{
public:
  virtual void socket_write(tcp::socket& socket, string request) = 0;
  virtual void socket_read_until(tcp::socket& socket, boost::asio::streambuf& response, const char* delim) = 0;
  virtual ~SocketInf() {}
};


class ServerSocketInf : public SocketInf
{
public:
  void socket_write(tcp::socket& socket, string request) {
    boost::asio::streambuf request_buf;
    std::ostream request_stream(&request_buf);
    request_stream << request;
    boost::asio::write(socket, request_buf);
  }
  void socket_read_until(tcp::socket& socket, boost::asio::streambuf& response, const char* delim) {
    boost::asio::read_until(socket, response, delim);
  }
  ~ServerSocketInf() {}
};


class ProxyHandler : public RequestHandler
{
public:
    ProxyHandler(SocketInf* socket_inf) : socket_inf_(socket_inf) {}
    static RequestHandler* create(const NginxConfig& config, const std::string& path);
    static RequestHandler* create(const NginxConfig& config, const std::string& path, SocketInf* socket_inf);
    
    std::unique_ptr<Response> HandleRequest(const Request& request);
    std::map<std::string, std::string> HTTPRequest();
    std::string make_string(boost::asio::streambuf& streambuf);
    std::string HTMLFilter(string body, string type);
private:
    std::string root_;
    std::string uri_;
    std::string host_;
    std::string path_;
    std::string port_;

    SocketInf* socket_inf_;
};
