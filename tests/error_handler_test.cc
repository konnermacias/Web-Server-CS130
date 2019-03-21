#include "error_handler.h"
#include <string>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "response.h"
#include "request.h"

class ErrorHandlerTest : public ::testing::Test 
{
protected:
    NginxConfig out_config_;
    NginxConfigParser parser_;
    RequestHandler* handler_;

    void init_error_handler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = ErrorHandler::create(out_config_, "/usr/src/project");
    }
    std::unique_ptr<Request> make_request(std::string raw_req) {
        return Request::request_handler(raw_req);
    }
    };

TEST_F(ErrorHandlerTest, successTest) {
  std::unique_ptr<Request> req = make_request("GET /missing HTTP/1.1\r\n\r\n");
  init_error_handler("../tests/configs/echo_server_config");
  
  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 404\r\nContent-Type: text/plain\r\nContent-Length: 57\r\n\r\n404: File not found on path. Please provide correct path.");
}

