#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "proxy_handler.h"

#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

RequestHandler* ProxyHandler::create(const NginxConfig& config, const string& path)
{
    ServerSocketInf* socket_inf = new ServerSocketInf();
    ProxyHandler* ph = new ProxyHandler(socket_inf);
    ph->root_ = path;
    ph->uri_ = config.GetAttribute("url");
    ph->host_ = config.GetAttribute("host");
    ph->path_ = config.GetAttribute("path");
    ph->port_ = config.GetAttribute("port");
    return ph;
};

RequestHandler* ProxyHandler::create(const NginxConfig& config, const string& path, SocketInf* socket_inf)
{
    ProxyHandler* ph = new ProxyHandler(socket_inf);
    ph->root_ = path;
    ph->uri_ = config.GetAttribute("url");
    ph->host_ = config.GetAttribute("host");
    ph->path_ = config.GetAttribute("path");
    ph->port_ = config.GetAttribute("port");
    return ph;
};

//TOCHECK: HandleRequest seems to be called twice per client request
unique_ptr<Response> ProxyHandler::HandleRequest(const Request& request)
{
    map<string, string> mapz = HTTPRequest();

    // create fresh response obj
    BOOST_LOG_TRIVIAL(trace) << "proxy handler building response for request...";
    unique_ptr<Response> response(new Response());

    // set response data
    response->SetStatus(Response::MOVED_TEMPORARILY);
    response->ReSetHeader("Content-Type",mapz["Content-Type"]);
    response->SetHeader("Content-Length", to_string(mapz["Body"].length()));
    response->SetBody(mapz["Body"]);
    BOOST_LOG_TRIVIAL(trace) << "Response built by Proxy Handler...";

    //Server monitoring 
    std::cout << "::HandlerMetrics:: handler:proxy" << std::endl;
    std::cout << "::ResponseMetrics:: response_code:302" << std::endl;
    

    return response;
};

string ProxyHandler::make_string(boost::asio::streambuf& streambuf)
{
    return {buffers_begin(streambuf.data()), buffers_end(streambuf.data())};
}

//this filter checks if the response is an html file, if it is, rewrite all absolute urls to 
//be served directly by the host_ specified in the config file.
string ProxyHandler::HTMLFilter(string body, string type)
{   
    //checks if type is html
    if(type.find("text/html") != string::npos)
    {
        //prefix absolute urls with http://<hostname>
        regex e2 ("\\b([a-zA-Z]*=\")(/.*)(\")");
        string http = "http://";
        string to_return = regex_replace (body,e2,"$1" + http + host_ + "$2" + "$3");
        return to_return;
    }
    //returns an unaltered response body
    return body;
}

map<string, string> ProxyHandler::HTTPRequest()
{
    map<string, string> to_return;
    to_return["Content-Type"] = "text/plain";

    boost::asio::io_service io_service;
    // Try each endpoint until we successfully establish a connection.
    tcp::socket socket(io_service);

    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host_, port_);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::connect(socket, endpoint_iterator);

    string request_str;
    request_str += "GET " + path_ + " HTTP/1.1\r\n";
    request_str += "Host: " + host_ + "\r\n";
    request_str += "Accept: */*\r\n";
    request_str += "Connection: close\r\n\r\n";

    // Send the request.
    socket_inf_->socket_write(socket, request_str);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;

    socket_inf_->socket_read_until(socket, response, "\r\n");

    string res_body;

    // Check that response is OK.
    std::istream response_stream(&response);

    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
        to_return["Body"] = "Invalid response\n";
        return to_return;
    }
    if (status_code != 200)
    {
        to_return["Body"] = ("Response returned with status code " + to_string(status_code) + "\n");
        return to_return;
    }

    // Read the response headers, which are terminated by a blank line.
    socket_inf_->socket_read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    int c_length_val = -1;
    string c_type_val = "";
    string c_length_key = "Content-Length:";
    string c_type_key = "Content-Type:";
    while (std::getline(response_stream, header) && header != "\r")
    {
        if (header.find(c_length_key) != string::npos)
        {
            c_length_val = stoi(header.substr(c_length_key.length()+1));
        }
        if (header.find(c_type_key) != string::npos)
        {
            c_type_val = header.substr(c_type_key.length()+1);
            to_return["Content-Type"] = c_type_val;
        }
    }

    // Write whatever content we already have to output.
    string last_portion = "";
    if (response.size() > 0)
    {
        last_portion = make_string(response);
        res_body += make_string(response);
    }

    // Read until EOF, writing data to output as we go.
    if (last_portion.length() != c_length_val)
    {
        boost::system::error_code error;
        while (true)
        {
            size_t n = boost::asio::read(socket, response, error);

            if (n == 0)
                break;
            
            res_body += make_string(response);
        }
    }

    //res_body is passed through ann HTML filter, to rewrite absolute urls
    to_return["Body"] = HTMLFilter(res_body, to_return["Content-Type"]);
    return to_return;
}
