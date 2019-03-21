#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>

#include "response.h"

Response::Response() {
    header_ = "";
}

void Response::SetStatus(const statuscode status_code)
{
    status_code_ = status_code;
}
void Response::SetHeader(const std::string& header_name, const std::string& header_value)
{
    const std::string CRLF = "\r\n";
    header_ += header_name + ": " + header_value + CRLF; 
}
void Response::ReSetHeader(const std::string& header_name, const std::string& header_value)
{
    const std::string CRLF = "\r\n";
    header_ = header_name + ": " + header_value + CRLF; 
}
void Response::SetBody(std::string body_value)
{
    body_ = body_value;
}
Response::statuscode Response::getStatusCode()
{
    return status_code_;
}

std::string Response::Output()
{   
    const std::string CRLF = "\r\n";

    //Build the full response message from pieces
    std::string status_line = "HTTP/1.1 " + std::to_string(status_code_) + CRLF; 

    return status_line + header_ + CRLF + body_;
};