#include "dispatcher.h" 
#include "request.h"
#include "handler_manager.h"
#include "request_handler.h"
#include "echo_handler.h"
#include "static_handler.h"
#include "status_handler.h"
#include "error_handler.h"
#include "gtest/gtest.h"

class DispatcherTest : public ::testing::Test 
{
  protected:
    NginxConfigParser parser_;
    NginxConfig out_config_;
    Dispatcher* dispatcher_;

    bool InitiateDispatcher(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        dispatcher_ = new Dispatcher(&out_config_);
        if (dispatcher_)
            return true;
        return false;
    }

    std::unique_ptr<Request> check_request(std::string raw_req)
    {
        return Request::request_handler(raw_req);
    }
};

TEST_F(DispatcherTest, DispatcherInitTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
}

TEST_F(DispatcherTest, DispatchFunctionTest1) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    Request req("GET / HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(&req), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionEchoTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /echo HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionStaticTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /static1/photo1.jpeg HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionHeatlhTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /health HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionStatusTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /status HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionMemeTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /meme/create HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchFunctionProxyTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /proxy2 HTTP/1.1\r\n\r\n");
    EXPECT_NE(dispatcher_->generateResponse(req.get()), nullptr);
}

TEST_F(DispatcherTest, DispatchGetRequestTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /echo HTTP/1.1\r\n\r\n");
    dispatcher_->generateResponse(req.get());
    EXPECT_EQ(dispatcher_->getRequest(), req.get());
}

TEST_F(DispatcherTest, DispatchGetResponseTest) 
{
    EXPECT_TRUE(InitiateDispatcher("../tests/configs/echo_server_config"));
    std::unique_ptr<Request> req = check_request("GET /echo HTTP/1.1\r\n\r\n");
    std::unique_ptr<Response> resp = dispatcher_->generateResponse(req.get());
    EXPECT_EQ(dispatcher_->getResponse(), resp.get());
}