#include <string>
#include <vector>
#include <fstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <sstream> 
#include "static_handler.h"
#include "config_parser.h"
#include <iostream>


RequestHandler* StaticHandler::create(const NginxConfig& config, const std::string& path)
{
    StaticHandler* sh = new StaticHandler();
    sh->uri_ = config.GetAttribute("url");
    sh->filedir_ = config.GetAttribute("location");
    sh->root_ = path;

    BOOST_LOG_TRIVIAL(trace) << "Static handler created";

    return sh;
}

std::unique_ptr<Response> StaticHandler::HandleRequest(const Request& request)
{
    BOOST_LOG_TRIVIAL(trace) << "Static Handler building response for request...";

    // uri: /static/somefile.html
    std::string full_url = request.uri_path();
    std::string filename = full_url.substr(uri_.length() + 1, full_url.length());
    std::string fileextension;
    std::string contenttype;

    int dot_index = filename.find(".");
    if (dot_index != std::string::npos)
        fileextension = filename.substr(dot_index + 1, filename.length() - dot_index - 1);

    // determine file directory
    // should be already populated when object is created. Delete as necessary
    // std::string filedir_ = parse_uri(request.uri_path());

    //read in file
    std::string image;
    
    //GCP uri_path
    std::ifstream ifs("static" + filedir_ + "/" + filename, std::ios::in | std::ios::binary);
    // local uri_path
    //std::ifstream ifs(".." + filedir_ + "/" + filename, std::ios::in | std::ios::binary);
   

    std::unique_ptr<Response> response(new Response());

    //if fail, give 404 error
    if (!ifs.is_open() || filename.length() == 0)
    {
        std::string error_msg = "404: File not found on uri_path. Please provide correct uri_path.";
        response->SetStatus(Response::NOT_FOUND);
        response->ReSetHeader("Content-Type", "text/plain");
        response->SetHeader("Content-Length", std::to_string(error_msg.length()));
        response->SetBody(error_msg);

        //Server monitoring 
        std::cout << "::HandlerMetrics:: handler:error" << std::endl;
        std::cout << "::ResponseMetrics:: response_code:404" << std::endl;

        return response;
    }
    char buf[512];
    while (ifs.read(buf, sizeof(buf)).gcount() > 0) {
        image.append(buf, ifs.gcount());
    }
    ifs.close();
    
    //TODO: Covert MIME handling to map
    //text file MIMES
    if (fileextension == "html")
        contenttype = "text/html";
    else if (fileextension == "htm")
        contenttype = "text/html";
    else if (fileextension == "css")
        contenttype = "text/css";
    else if (fileextension == "csv")
        contenttype = "text/csv";
    else if (fileextension == "txt")
        contenttype = "text/plain";
    else if (fileextension == "js")
        contenttype = "text/javascript";
    else if (fileextension == "xml")
        contenttype = "text/xml";

    //image file MIMES
    else if (fileextension == "jpg" || fileextension == "jpeg")
        contenttype = "image/jpeg";
    else if (fileextension == "png")
        contenttype = "image/png";
    else if (fileextension == "gif")
        contenttype = "image/gif";
    else if (fileextension == "tif" || fileextension == "tiff")
        contenttype = "image/tiff";

    //application file MIMES
    else if (fileextension == "pdf")
        contenttype = "application/pdf";
    else if (fileextension == "zip")
        contenttype = "application/zip";
    else if (fileextension == "rtf")
        contenttype = "application/rtf";
    else if (fileextension == "json")
        contenttype = "application/json";
    else if (fileextension == "doc")
        contenttype = "application/msword";

    else

    //if fail, give 404 error
    { 
        std::string error_msg = "404: File not found on uri_path. Please provide correct uri_path.";
        response->SetStatus(Response::NOT_FOUND);
        response->ReSetHeader("Content-Type", "text/plain");
        response->SetHeader("Content-Length", std::to_string(error_msg.length()));
        response->SetBody(error_msg);

        //Server monitoring 
        std::cout << "::HandlerMetrics:: handler:error" << std::endl;
        std::cout << "::ResponseMetrics:: response_code:404" << std::endl;

        return response;
    }

    //build a correct response 
    response->SetStatus(Response::OK);
    response->SetHeader("Content-Type", contenttype);
    response->SetHeader("Content-Length", std::to_string(image.length()));
    response->SetBody(image);
    BOOST_LOG_TRIVIAL(trace) << "Response built by static handler...";

    //Server monitoring 
    std::cout << "::HandlerMetrics:: handler:static" << std::endl;
    std::cout << "::ResponseMetrics:: response_code:200" << std::endl;
    
    return response;
};