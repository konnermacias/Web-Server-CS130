#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <memory>
#include <sstream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include "request.h"

Request::Request(std::string req) : req_(req) {}

std::unique_ptr<Request> Request::request_handler(std::string raw_req)
{
    std::unique_ptr<Request> request(new Request(raw_req));

    if (request->parse_request())
    {
        return request;
    }
    else
    {
        //Malformed request
        request->method_ = "GET";
        request->uri_path_ = "/bad";
        request->http_version_ = "HTTP/1.1";
        return request;
    }
}

bool Request::check_first_request_line(std::string req_line)
{
    // check for correct formatting
    std::vector<std::string> req_comps;
    boost::split(req_comps, req_line, boost::is_any_of(" "), boost::token_compress_on);
    
    // check for valid size
    const int REQUIRED_COMP_SIZE = 3;
    if (req_comps.size() != REQUIRED_COMP_SIZE) return false;

    // check valid method
    if (req_comps[0] != "GET" && req_comps[0] != "POST") return false;

    // check correct HTTP version
    if (req_comps[2].find("HTTP/1.1") == std::string::npos) return false;

    this->method_ = req_comps[0];
    this->uri_path_ = req_comps[1];
    this->http_version_ = req_comps[2];

    return true;
}

bool Request::parse_request()
{
    BOOST_LOG_TRIVIAL(trace) << "Parsing input data...";
    // where I will read in data and split
    std::vector<std::string> req_lines;
    bool CRLF_FLAG = true;
    int field_index = req_.find("\r\n\r\n");

    if (field_index == std::string::npos)
    {
        // if \r\n\r\n fails, try \n
        field_index = req_.find('\n');
        if (field_index == std::string::npos) 
        {
            std::cout << "Error!: " << field_index <<  std::endl;
            BOOST_LOG_TRIVIAL(error) << "Error finding request field before request body";
            return false;
        }
        
        CRLF_FLAG = false;   
    }
    
    // split request content into vector of its lines
    if (CRLF_FLAG)
    {
        boost::split(req_lines, req_, boost::is_any_of("\r\n\r\n"), boost::token_compress_on);
    }
    else
    {
        boost::split(req_lines, req_, boost::is_any_of("\n"), boost::token_compress_on);
    }
    
    // check first line of request
    BOOST_LOG_TRIVIAL(trace) << "Checking first request line...";
    if (!check_first_request_line(req_lines[0]))
    {
        std::cout << req_lines[0] << std::endl;
        std::cout << "Error parsing first request line" << std::endl;
        BOOST_LOG_TRIVIAL(error) << "Error parsing first request line";
        return false;
    }

    // create out header key value pairs
    // NOTE: We are using size() - 1 bc the last elm should be nl
    for (size_t i = 1; i < req_lines.size() - 1; i++)
    {
        // find index to break into header key value pair
        size_t colon_index = req_lines[i].find(": ");
        if (colon_index == std::string::npos)
        {
            // rn disregard a bad line
            continue;
        }

        // add corresponding header pair
        std::string header_key = req_lines[i].substr(0, colon_index);
        std::string header_value = req_lines[i].substr(colon_index + 2);
        std::pair<std::string,std::string> header_field(header_key, header_value);
        this->header_fields_.push_back(header_field);
    }
    // assign body
    this->body_ = req_lines[req_lines.size()-1];
    BOOST_LOG_TRIVIAL(trace) << "Finished adding heading pairs...";
    return true;
}

std::string Request::getReqRaw() const{
    return req_;
}

std::string Request::method() const
{
    return method_;
}

std::string Request::uri_path() const
{
    return uri_path_;
}

std::string Request::http_version() const
{
    return http_version_;
}

std::string Request::body() const
{
    return body_;
}