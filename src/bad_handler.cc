#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "bad_handler.h"


RequestHandler* BadHandler::create(const NginxConfig& config, const std::string& path)
{
    BadHandler* bad = new BadHandler();
    bad->root_ = path;

    BOOST_LOG_TRIVIAL(trace) << "Bad handler created";

    return bad;
};


std::unique_ptr<Response> BadHandler::HandleRequest(const Request& request)
{
    BOOST_LOG_TRIVIAL(trace) << "Bad handler building response for request...";

    std::unique_ptr<Response> response(new Response());
    std::string body_ = "400:Bad Request";

    response->SetStatus(Response::BAD_REQUEST);
    response->ReSetHeader("Content-Type","text/plain");
    response->SetHeader("Content-Length", std::to_string(body_.length()));
    response->SetBody(body_);
    
    BOOST_LOG_TRIVIAL(trace) << "Response built by bad handler...";

    //Server monitoring 
    std::cout << "::HandlerMetrics:: handler:bad" << std::endl;
    std::cout << "::ResponseMetrics:: response_code:400" << std::endl;

    return response;
};