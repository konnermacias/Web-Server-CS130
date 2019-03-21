#pragma once

#include <string>
#include <vector>
#include "request.h"
#include "response.h"
#include "config_parser.h"

class Request;
class Response;

class RequestHandler
{
  public: 
    static RequestHandler* create(const NginxConfig& config, const std::string& path);
    virtual std::unique_ptr<Response> HandleRequest(const Request& request) = 0;

  protected: 
    std::string path_;
    NginxConfig config_;
};