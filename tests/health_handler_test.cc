#include "health_handler.h"
#include <string>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "response.h"
#include "request.h"

class HealthHandlerTest : public ::testing::Test 
{
protected:
    NginxConfig out_config_;
    NginxConfigParser parser_;
    RequestHandler* handler_;

    void init_health_handler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = HealthHandler::create(out_config_, "/usr/src/project");
    }
    std::unique_ptr<Request> make_request(std::string raw_req) {
        return Request::request_handler(raw_req);
    }
    };

TEST_F(HealthHandlerTest, successTest) {
  std::unique_ptr<Request> req = make_request("GET /health HTTP/1.1\r\n\r\n");
  init_health_handler("../tests/configs/echo_server_config");
  
  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::OK);

}

