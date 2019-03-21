#include <memory>
#include "gtest/gtest.h"
#include "request.h"

// define fixture
class RequestTests : public ::testing::Test
{
  protected:
    std::unique_ptr<Request> check_request(std::string raw_req)
    {
        return Request::request_handler(raw_req);
    }
};


TEST_F(RequestTests, MethodTypeCheckCorrectness)
{
    // TODO: add other method type checks
    std::unique_ptr<Request> req = check_request("GET / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req->method(), "GET");
    EXPECT_EQ(req->uri_path(), "/");
    EXPECT_EQ(req->http_version(), "HTTP/1.1");
    EXPECT_EQ(req->body(), "");
    
    // test against POST request
    req = check_request("POST / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req->method(), "POST");
    EXPECT_EQ(req->uri_path(), "/");
    EXPECT_EQ(req->http_version(), "HTTP/1.1");

    // test against invalid method type
    req = check_request("KON / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req->uri_path(), "/bad");
}

TEST_F(RequestTests, HTTPVersionCorrectness)
{
    // check for simple case
    std::unique_ptr<Request> req = check_request("GET / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(req->http_version(), "HTTP/1.1");
    // check against wrong HTTP number
    req = check_request("GET / HTTP/1.0\r\n\r\n");
    EXPECT_EQ(req->uri_path(),"/bad");
    // check against blank HTTP option
    req = check_request("GET / HTTP/\r\n\r\n");
    EXPECT_EQ(req->uri_path(),"/bad");
}

TEST_F(RequestTests, HTTPBodyValueParsing)
{
    std::string request = "GET / HTTP/1.1\r\n\r\nHost: 35.185.231.37\r\nConnection: keep-alive\r\n\r\n";
    std::unique_ptr<Request> req = check_request(request);
    EXPECT_EQ(req->http_version(), "HTTP/1.1");
}

TEST_F(RequestTests, HTTPBodyValueParsingInvalidBody)
{
    std::string request = "GET / HTTP/1.1\r\n\r\nHost 35.185.231.37\r\nConnection keep-alive\r\n\r\n";
    std::unique_ptr<Request> req = check_request(request);
    EXPECT_EQ(req->http_version(), "HTTP/1.1");
}

TEST_F(RequestTests, OnlyOneLF)
{
    std::string request = "GET / HTTP/1.1\n";
    std::unique_ptr<Request> req = check_request(request);
    EXPECT_EQ(req->http_version(), "HTTP/1.1");
}

TEST_F(RequestTests, InvalidInputTest)
{
    // did not specify a method type and incorrect args
    std::unique_ptr<Request> req = check_request("/ HTTP/1.1");
    EXPECT_EQ(req->uri_path(),"/bad");
    // did not contain: '\r\n\r\n'
    req = check_request("GET / HTTP/1.1");
    EXPECT_EQ(req->uri_path(),"/bad");
    // check against empty input
    req = check_request("");
    EXPECT_EQ(req->uri_path(),"/bad");
}


TEST_F(RequestTests, MultilineTest)
{
    // check for two lines
     std::unique_ptr<Request> req = check_request("GET / HTTP/1.1\r\n\r\nHost: 35.185.231.37\r\n\r\n");
}

