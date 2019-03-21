#include <memory>
#include <iostream>
#include <streambuf>
#include <istream>
#include "session.h"
#include "request.h"
#include "response.h"
#include "echo_handler.h"
#include "error_handler.h"
#include "static_handler.h"
#include "status_obj.h"
#include "dispatcher.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/system/system_error.hpp>

#include <vector>
#include <boost/bind.hpp>


void session::start()
{
    BOOST_LOG_TRIVIAL(trace) << "Session has started...";
    try { BOOST_LOG_TRIVIAL(info) << "New connection from IP: " << socket_.remote_endpoint().address().to_string(); }
    catch(boost::system::system_error const& e) { std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n"; }

    //Listen in for requests
    boost::asio::async_read_until(socket_, buffer_, "\r\n\r\n",
        boost::bind(&session::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void session::handle_read(const boost::system::error_code& error,
    size_t bytes_transferred)
{
    if (error == boost::asio::error::eof)
    {
        BOOST_LOG_TRIVIAL(trace) << "EOF received...";
        try { BOOST_LOG_TRIVIAL(info) << "Dropped connection from IP: " << socket_.remote_endpoint().address().to_string() << "..."; }
        catch(boost::system::system_error const& e) { std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n"; }

        return;
    }
    if (!error)
    {   
        std::string httpresponse;

        std::ostringstream oss;
        oss << &buffer_;
        std::string data_ = oss.str();
        
        BOOST_LOG_TRIVIAL(trace) << "Sending data to request handler...";

        std::unique_ptr<Request> req = Request::request_handler(data_);

        // view data members of request
        BOOST_LOG_TRIVIAL(trace) << "Info about request{ "
                                    << "Method: " << req->method()
                                    << ", URI Path: " << req->uri_path()
                                    << ", HTTP Version: " << req->http_version()
                                    << " }";

        //Server monitoring 
        std::cout << "::PathMetrics:: path:"<< req->uri_path() << std::endl;

        // construct dispatcher and handle incoming request
        BOOST_LOG_TRIVIAL(info) << "Constructing dispatcher...";
        Dispatcher dispatcher(config_);
        std::unique_ptr<Response> resp = dispatcher.generateResponse(req.get());

        // check if dispatch returns a valid response pointer
        if (resp)
        {
            // get resp object and set httpresponse string
            BOOST_LOG_TRIVIAL(info) << "Dispatcher generating appropriate response...";
            httpresponse = resp->Output();
            
            // add data to status database
            BOOST_LOG_TRIVIAL(trace) << "Adding url and response code into Status Database...";
            StatusObject::addStatusEntry(req->uri_path(), std::to_string(resp->getStatusCode()));
            BOOST_LOG_TRIVIAL(trace) << "...";
        }
        
        else
        {
            httpresponse = "";
        }
        

        //Write response to client
        boost::asio::async_write(socket_,
            boost::asio::buffer(httpresponse.c_str(), httpresponse.length()),
            boost::bind(&session::handle_write, shared_from_this(),
            boost::asio::placeholders::error));

    }
    else
    {
        BOOST_LOG_TRIVIAL(error) << "async_read_until failed...";
        delete this;
    }
}

void session::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        try { BOOST_LOG_TRIVIAL(info) << "Writing response to IP: " << socket_.remote_endpoint().address().to_string() << "..."; }
        catch(boost::system::system_error const& e) { std::cout << e.what() << ": " << e.code() << " - " << e.code().message() << "\n"; }
        
        //If there's an error shutdown the communications
        boost::system::error_code ignored_ec;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);

    }
    else
    {
      delete this;
    }
}