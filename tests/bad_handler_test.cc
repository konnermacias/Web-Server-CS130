#include "bad_handler.h"
#include <string>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "response.h"
#include "request.h"

class BadHandlerTest : public ::testing::Test 
{
protected:
    NginxConfig out_config_;
    NginxConfigParser parser_;
    RequestHandler* handler_;

    void init_bad_handler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = BadHandler::create(out_config_, "/usr/src/project");
    }
    std::unique_ptr<Request> make_request(std::string raw_req) {
        return Request::request_handler(raw_req);
    }
    };

TEST_F(BadHandlerTest, badGetTest) {
  std::unique_ptr<Request> req = make_request("GE /blah HTTP/1.0\r\n\r\n");
  init_bad_handler("../tests/configs/echo_server_config");
  
  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::BAD_REQUEST);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 400\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\n400:Bad Request");
}

TEST_F(BadHandlerTest, badHTTPTest) {
  std::unique_ptr<Request> req = make_request("GET /blah TTP/1.0\r\n\r\n");
  init_bad_handler("../tests/configs/echo_server_config");
  
  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::BAD_REQUEST);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 400\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\n400:Bad Request");
}

TEST_F(BadHandlerTest, extratextTest) {
  std::unique_ptr<Request> req = make_request("GET /blah HTTP/1.0asdf\r\n\r\n");
  init_bad_handler("../tests/configs/echo_server_config");
  
  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::BAD_REQUEST);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 400\r\nContent-Type: text/plain\r\nContent-Length: 15\r\n\r\n400:Bad Request");
}
