#pragma once

#include <string>
#include <vector>
#include "request.h"
#include "response.h"
#include "request_handler.h"
#include "config_parser.h"
#include "status_obj.h"

class StatusHandler : public RequestHandler
{
public:   
    //create an instance of the handler
    static RequestHandler*  create(const NginxConfig& config, const std::string& path);
    
    //Handles the request and generates a response
    std::unique_ptr<Response> HandleRequest(const Request& request);

private:
    std::string root_;
    std::string uri_;
};