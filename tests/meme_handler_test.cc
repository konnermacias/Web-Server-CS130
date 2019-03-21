#include <string>
#include <fstream>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include "session.h"
#include "response.h"
#include "request.h"
#include "dispatcher.h"
#include "handler_manager.h"
#include "meme_handler.h"

class MemeHandlerTest : public ::testing::Test 
{
    protected:
        boost::asio::io_service io_service_;
        NginxConfig out_config_;
        NginxConfigParser parser_;
        RequestHandler* handler_;

    void init_meme_handler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = MemeHandler::create(out_config_, "/usr/src/project");
    }

    std::unique_ptr<Request> make_request(std::string raw_req) {
        return Request::request_handler(raw_req);
    } 

    std::string get_req_string(std::string request_file_str) 
    {
        std::stringstream str;
        std::ifstream stream(request_file_str);
        if (stream.is_open()) 
        {
            while(stream.peek() != EOF)
                str << (char) stream.get();
            stream.close();
            return str.str();
        }
    }

    std::map<std::string, std::string> SQL_GetMemeFromDatabase(std::string config_file, std::string id) {
        parser_.Parse(config_file.c_str(), &out_config_);
        MemeHandler* meme_handler_ = (MemeHandler*) MemeHandler::create(out_config_, "/usr/src/project");
        MemeHandler_friend* meme_handler_friend_;
        return meme_handler_friend_->call_GetMemeFromDatabase(meme_handler_, id);
    }
};

//-----------Create Tests -----------//
TEST_F(MemeHandlerTest, GoodCreatePageTest) {
    //std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/create_request"));
    std::unique_ptr<Request> req = make_request("GET /meme/create HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

//-----------List Tests -----------//
TEST_F(MemeHandlerTest, ListPageTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/list HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}


//-----------View Tests -----------//
TEST_F(MemeHandlerTest, BadViewPageTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/view HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND); 
}

TEST_F(MemeHandlerTest, PostAndViewPageTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request"));
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    std::unique_ptr<Request> req2 = make_request("GET /meme/view?id=10003 HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp2 = handler_->HandleRequest(*(req2.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

//-----------Post Tests -----------//
TEST_F(MemeHandlerTest, PostTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}


TEST_F(MemeHandlerTest, EncodeTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request3"));
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

TEST_F(MemeHandlerTest, PostDBFailTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request"));
    init_meme_handler("../tests/configs/bad_meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

TEST_F(MemeHandlerTest, PostNoParameterTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request4"));
    init_meme_handler("../tests/configs/bad_meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

//-----------Error Tests -----------//
TEST_F(MemeHandlerTest, ErrorTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/missing HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

TEST_F(MemeHandlerTest, BadIDTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/view?id=99999 HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

TEST_F(MemeHandlerTest, NotNumberIDTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/view?id=abcde HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

//-----------List Tests -----------//
TEST_F(MemeHandlerTest, ListTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/list HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

TEST_F(MemeHandlerTest, FilledListTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

//-----------Delete Tests -----------//

TEST_F(MemeHandlerTest, ValidDeleteTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    // id = 10000 must exist <- should
    req = make_request(get_req_string("../tests/meme_handler_tests/del_request"));
    init_meme_handler("../tests/configs/meme_config");

    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(1,1);
}

TEST_F(MemeHandlerTest, WrongTokenDeleteTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    // id = 10000 must exist <- should
    req = make_request(get_req_string("../tests/meme_handler_tests/del_request3"));
    init_meme_handler("../tests/configs/meme_config");

    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(1,1);
}

TEST_F(MemeHandlerTest, BadDeleteTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/delete HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));

    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND); 
}


/*
    This shouldn't happen if the view?id= is correct
TEST_F(MemeHandlerTest, InvalidIdDeleteTest) {
    // id = 10000 must exist
    std::unique_ptr<Request> req = make_request("GET /meme/delete?id=1fewf HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}
*/

//-----------Edit Tests------------//

TEST_F(MemeHandlerTest, ValidEditTest) {


    // id = 10000 must exist <- should
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/edit_request"));
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    // id = 10000 must exist <- should
    req = make_request(get_req_string("../tests/meme_handler_tests/edit_post_req"));
    init_meme_handler("../tests/configs/meme_config");

    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

//-----------Search Tests----------//
TEST_F(MemeHandlerTest, SearchTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list?q=&sort=a-z HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list?q=&sort=z-a HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list?q=&sort=newest HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list?q=&sort=oldest HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    req = make_request("GET /meme/list?q=&sort=id HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);
}

TEST_F(MemeHandlerTest, BadSearchTest) {
    std::unique_ptr<Request> req = make_request("GET /meme/list?q=&bad=bad HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

//-----------SQL Tests -----------//
TEST_F(MemeHandlerTest, SQLInjectionTest) {

    std::unique_ptr<Request> req2 = make_request("GET /meme/viewid=10000;DROP+ALL+TABLES HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp2 = handler_->HandleRequest(*(req2.get()));

    EXPECT_EQ(resp2->getStatusCode(), Response::NOT_FOUND); 

    std::unique_ptr<Request> req3 = make_request("GET /meme/viewid=10000;DROP+TABLE+MEME_HISTORY HTTP/1.1\r\n\r\n");
    init_meme_handler("../tests/configs/meme_config");

    std::unique_ptr<Response> resp3 = handler_->HandleRequest(*(req3.get()));

    EXPECT_EQ(resp3->getStatusCode(), Response::NOT_FOUND); 

}

TEST_F(MemeHandlerTest, DatabaseFunctionTest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request2"));
    init_meme_handler("../tests/configs/meme_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK);

    std::map<std::string, std::string> meme = SQL_GetMemeFromDatabase("../tests/configs/meme_config", "10002");

    EXPECT_NE(meme["NAME"], "");
}

//-----------Bad Meme Save Directory--------------//
TEST_F(MemeHandlerTest, BadDatabaseCreateTest) {
    //std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/create_request"));
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/post_request"));
    init_meme_handler("../tests/configs/bad_meme_config");
    
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

TEST_F(MemeHandlerTest, BadDatabaseDeleteTest) {
    // id = 10000 must exist <- should
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/del_request"));
    init_meme_handler("../tests/configs/bad_meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}

TEST_F(MemeHandlerTest, BadDatabaseEditTest) {
    // id = 10000 must exist <- should
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/meme_handler_tests/edit_post_req"));
    init_meme_handler("../tests/configs/bad_meme_config");

    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND);
}