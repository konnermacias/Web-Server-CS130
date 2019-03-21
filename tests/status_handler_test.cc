#include "status_handler.h"
#include <string>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "response.h"
#include "request.h"

class StatusHandlerTest : public ::testing::Test 
{
protected:
    NginxConfig out_config_;
    NginxConfigParser parser_;
    RequestHandler* handler_;

    void init_status_handler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = StatusHandler::create(out_config_, "/usr/src/project");
    }
    std::unique_ptr<Request> make_request(std::string raw_req) {
        return Request::request_handler(raw_req);
    }
};

TEST_F(StatusHandlerTest, successTest) {
    StatusObject::addStatusEntry("this","one");
    StatusObject::url_handlers_map_.insert(std::pair <std::string, std::string>("another","one"));
    std::unique_ptr<Request> req = make_request("GET /status\r\n\r\n");
    init_status_handler("../tests/configs/echo_server_config");
    
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}


