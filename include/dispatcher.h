#pragma once
#include <map>
#include "config_parser.h"
#include "request.h"
#include "response.h"

class Dispatcher {
  public:
    Dispatcher (NginxConfig* config);
    std::unique_ptr<Response> generateResponse(Request* req);
    Request* getRequest();
    Response* getResponse();
  private:
    Request* req_;
    Response* resp_;
    std::string root_;
    std::map<std::string, NginxConfig*> configTable_;
    std::map<std::string, std::string> handlerTable_;
};