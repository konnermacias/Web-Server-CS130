#include "dispatcher.h" 
#include "handler_manager.h"
#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "status_handler.h"
#include "error_handler.h"
#include "meme_handler.h"
#include "health_handler.h"
#include "bad_handler.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/system/system_error.hpp>

Dispatcher::Dispatcher (NginxConfig* config) : req_(new Request("")), resp_(new Response())
{
    root_ = config->GetRoot();
    config->ParseString(0, configTable_, handlerTable_);
}

std::unique_ptr<Response> Dispatcher::generateResponse(Request *req)
{
    /*
        Determines name, and fires off HandlerManager to get a RequestHandler object.
        This object will handle the request.
    */

    // read full url
    std::string full_url = req->uri_path();
    int i = 1;
    for (i; i < full_url.length(); i++)
    {
        if (full_url[i] == '/')
            break;
    }
    
    // extract uri 
    std::string uri = full_url.substr(0,i);

    // check if handler type exists for this url
    std::map<std::string,std::string>::iterator it = handlerTable_.find(uri);
    if (it != handlerTable_.end())
    {
        // assign 
        HandlerManager handlermanager_;
        std::string name = handlerTable_.find(uri)->second;
        NginxConfig* nginxconfig = configTable_.find(uri)->second;
        std::string path = root_;

        // now create appropriate request handler
        BOOST_LOG_TRIVIAL(trace) << "Building appropriate request handler...";
        std::unique_ptr<RequestHandler> handler = handlermanager_.createByName(name, *nginxconfig, path);
        BOOST_LOG_TRIVIAL(info) << "Handling request...";
        std::unique_ptr<Response> resp =  handler->HandleRequest(*req);

        // assign req_ and resp_ objects TODO: delete?
        req_ = req;
        resp_ = resp.get();
        return resp;
    }
    else 
    {
        // assign 
        HandlerManager handlermanager_;
        std::string path = root_;
        NginxConfig emptyconfig;

        // now create appropriate request handler
        BOOST_LOG_TRIVIAL(trace) << "Building appropriate request handler...";
        std::unique_ptr<RequestHandler> handler = handlermanager_.createByName("error", emptyconfig, path);
        BOOST_LOG_TRIVIAL(info) << "Handling request...";
        std::unique_ptr<Response> resp =  handler->HandleRequest(*req);

        // assign req_ and resp_ objects TODO: delete?
        req_ = req;
        resp_ = resp.get();
        return resp;
    }
}

Request* Dispatcher::getRequest()
{
    return req_;
}

Response* Dispatcher::getResponse()
{
    return resp_;
}