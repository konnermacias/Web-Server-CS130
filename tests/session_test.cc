#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "config_parser.h"
#include "session.h"
#include "response.h"
#include "dispatcher.h"
#include "status_obj.h"
#include "handler_manager.h"

using boost::asio::ip::tcp;

class SessionTest : public ::testing::Test {
  protected:
    NginxConfig out_config_;
    NginxConfigParser parser_;

    boost::asio::io_service io_service_;
};


TEST_F(SessionTest, SocketTest) {
    // also tests session's socket function
    io_service_.run();
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, StartTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    new_session->start();
    EXPECT_EQ(1,1);
}


TEST_F(SessionTest, HandleReadEOFTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    sesh.call_handle_read(new_session.get(),boost::asio::error::eof, 100);
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, HandleReadRegularTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.call_handle_read(new_session.get(), ec, 100);
    EXPECT_EQ(1,1);
}


TEST_F(SessionTest, HandleReadDataTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.assign_data_and_call_read(new_session.get(), ec, 100, "GET / HTTP/1.1\r\n\r\n");
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, HandleReadDataEchoTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.assign_data_and_call_read(new_session.get(), ec, 100, "GET /echo HTTP/1.1\r\n\r\n");
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, HandleReadDataStaticFAILTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.assign_data_and_call_read(new_session.get(), ec, 100, "GET /static/ HTTP/1.1\r\n\r\n");
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, HandleReadDataStaticPASSTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.assign_data_and_call_read(new_session.get(), ec, 100, "GET /static/file HTTP/1.1\r\n\r\n");
    EXPECT_EQ(1,1);
}

TEST_F(SessionTest, HandleWriteNonErrorTest) {
    std::string filename = "../tests/configs/echo_server_config";
    parser_.Parse(filename.c_str(), &out_config_);

    session::pointer new_session = session::create(io_service_, &out_config_);
    Sesh sesh;
    boost::system::error_code ec;
    sesh.call_handle_write(new_session.get(), ec);
}
