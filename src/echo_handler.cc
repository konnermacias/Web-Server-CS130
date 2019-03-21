#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "echo_handler.h"


RequestHandler* EchoHandler::create(const NginxConfig& config, const std::string& path)
{
    EchoHandler* eh = new EchoHandler();
    eh->root_ = path;
    eh->uri_ = config.GetAttribute("url");

    BOOST_LOG_TRIVIAL(trace) << "Echo handler created";

    return eh;
};


std::unique_ptr<Response> EchoHandler::HandleRequest(const Request& request)
{
    BOOST_LOG_TRIVIAL(trace) << "Echo handler building response for request...";

    std::unique_ptr<Response> response(new Response());
    std::string body_ = std::string(request.getReqRaw());

    response->SetStatus(Response::OK);
    response->ReSetHeader("Content-Type","text/plain");
    response->SetHeader("Content-Length", std::to_string(body_.length()));
    response->SetBody(body_);
    
    BOOST_LOG_TRIVIAL(trace) << "Response built by echo handler...";

    //Server monitoring 
    std::cout << "::HandlerMetrics:: handler:echo" << std::endl;
    std::cout << "::ResponseMetrics:: response_code:200" << std::endl;

    return response;
};