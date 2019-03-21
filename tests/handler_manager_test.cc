#include "handler_manager.h"
#include "request.h"
#include "response.h"
#include "server.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include <typeinfo>

class HandlerManagerTest : public ::testing::Test {

    protected: 
        HandlerManager handlermanager_;
        NginxConfig* nginxconfig;

        std::unique_ptr<RequestHandler> check(std::string name, NginxConfig* nginxconfig, std::string path)
        {
            std::unique_ptr<RequestHandler> handler = handlermanager_.createByName(name, *nginxconfig, path);
            return handler;
        }
};


TEST_F(HandlerManagerTest, EchoTest)
{
    std::string input = "echo";
    std::string fakepath = "/echo";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, StaticTest)
{
    std::string input = "static";
    std::string fakepath = "/static";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, ErrorTest)
{
    std::string input = "error";
    std::string fakepath = "/error";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, StatusTest)
{
    std::string input = "status";
    std::string fakepath = "/status";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, FakeTest)
{
    std::string input = "fake";
    std::string fakepath = "/fake";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, ProxyTest)
{
    std::string input = "proxy";
    std::string fakepath = "/proxy";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

TEST_F(HandlerManagerTest, MemeTest)
{
    std::string input = "meme";
    std::string fakepath = "/meme";
    bool what = (typeid(check(input, nginxconfig, fakepath)) == typeid(std::unique_ptr<RequestHandler>));
    EXPECT_TRUE(what);
}

