#include <string>
#include <vector>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include "health_handler.h"


RequestHandler* HealthHandler::create(const NginxConfig& config, const std::string& path)
{
    HealthHandler* he = new HealthHandler();
    he->root_ = path;
    he->uri_ = config.GetAttribute("url");

    BOOST_LOG_TRIVIAL(trace) << "Health handler created";

    return he;
};


std::unique_ptr<Response> HealthHandler::HandleRequest(const Request& request)
{
    BOOST_LOG_TRIVIAL(trace) << "Health handler building response for request...";

    std::unique_ptr<Response> response(new Response());
    std::string body_ = "OK";

    response->SetStatus(Response::OK);
    response->ReSetHeader("Content-Type","text/plain");
    response->SetHeader("Content-Length", std::to_string(body_.length()));
    response->SetBody(body_);
    
    BOOST_LOG_TRIVIAL(trace) << "Response built by health handler...";

    //Server monitoring 
    std::cout << "::HandlerMetrics:: handler:health" << std::endl;
    std::cout << "::ResponseMetrics:: response_code:200" << std::endl;

    return response;
};