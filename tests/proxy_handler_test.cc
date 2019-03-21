#include <string>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "response.h"
#include "request.h"
#include "proxy_handler.h"


class MockSocketInf : public SocketInf
{
public:
  MOCK_METHOD2(socket_write, void(tcp::socket& socket, std::string request));
  MOCK_METHOD3(socket_read_until, void(tcp::socket& socket, boost::asio::streambuf& response, const char* delim));
};


class ProxyHandlerTest : public ::testing::Test 
{
protected:
  boost::asio::io_service io_service_;
  NginxConfig out_config_;
  NginxConfigParser parser_;
  RequestHandler* handler_;
  MockSocketInf socket_inf_;

  void init_proxy_handler(std::string config_file) {
    parser_.Parse(config_file.c_str(), &out_config_);
    handler_ = ProxyHandler::create(out_config_, "/usr/src/project");
  }

  void init_mock_proxy_handler(std::string config_file) {
    parser_.Parse(config_file.c_str(), &out_config_);
    handler_ = ProxyHandler::create(out_config_, "/usr/src/project", &socket_inf_);
  }

  std::unique_ptr<Request> make_request(std::string raw_req) {
    return Request::request_handler(raw_req);
  }
};

TEST_F(ProxyHandlerTest, successTest) {
  std::unique_ptr<Request> req = make_request("GET /proxy1 HTTP/1.1\r\n\r\n");
  init_proxy_handler("../tests/configs/echo_server_config");
  //string request = "GET /static_1/a.txt HTTP/1.1\r\nHost: www3.coolcoders.cs130.org\r\nAccept: *\r\nConnection: close\r\n\r\n";

  std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
  
  EXPECT_EQ(resp->getStatusCode(), Response::MOVED_TEMPORARILY);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 302\r\nContent-Type: text/plain\r\r\nContent-Length: 5\r\n\r\ntest\n");
}

TEST_F(ProxyHandlerTest, mockFailTest) {
  std::unique_ptr<Request> req = make_request("GET /proxy1 HTTP/1.1\r\n\r\n");
  init_mock_proxy_handler("../tests/configs/echo_server_config");
  //string request = "GET /static_1/a.txt HTTP/1.1\r\nHost: www3.coolcoders.cs130.org\r\nAccept: *\r\nConnection: close\r\n\r\n";
  
  //EXPECT_CALL(socket_inf_, socket_write(::testing::_, request));
  EXPECT_CALL(socket_inf_, socket_read_until(::testing::_, ::testing::_, ::testing::_));
  
  std::unique_ptr<Response> resp =handler_->HandleRequest(*(req.get()));

  EXPECT_EQ(resp->getStatusCode(), Response::MOVED_TEMPORARILY);
  EXPECT_EQ(resp->Output(), "HTTP/1.1 302\r\nContent-Type: text/plain\r\nContent-Length: 17\r\n\r\nInvalid response\n");
}

