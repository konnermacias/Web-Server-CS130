#pragma once

#include <string>
#include <vector>

//Adapted from https://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/example/cpp03/http/server/reply.cpp
class Response 
{
  public: 
    Response();
    enum statuscode
    {
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NO_CONTENT = 204,
        MULTIPLE_CHOICES = 300,
        MOVED_PERMANENTLY = 301,
        MOVED_TEMPORARILY = 302,
        NOT_MODIFIED = 304,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        UNSUPPORTED_MEDIA_TYPE = 415,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503
    };

    //set methods
    void SetStatus(const statuscode status_code);
    void SetHeader(const std::string& header_name, const std::string& header_value);
    void ReSetHeader(const std::string& header_name, const std::string& header_value);
    void SetBody(std::string body_value);

    //Get methods
    statuscode getStatusCode();
    std::string Output();

private:
    statuscode status_code_;
    std::string header_;
    std::string body_;
    int statuslinesize_;
    int headersize_;
    int bodysize_;
};