#include <string>
#include <fstream>
#include <streambuf>
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "session.h"
#include "response.h"
#include "request.h"
#include "dispatcher.h"
#include "status_obj.h"
#include "handler_manager.h"
#include "static_handler.h"

class StaticHandlerTest : public ::testing::Test 
{
  protected:
    boost::asio::io_service io_service_;
    NginxConfig out_config_;
    NginxConfigParser parser_;
    RequestHandler* handler_;

    void read_request_file(std::string request_file_str) 
    {
        const char* a_request = file_to_string(request_file_str);
        session* s = new session(io_service_, &out_config_);
        Sesh sesh;
        boost::system::error_code ec;
        sesh.assign_data_and_call_read(s, ec, 100, a_request);
    }

    const char* file_to_string(std::string request_file_str) 
    {
        std::stringstream str;
        std::ifstream stream(request_file_str);
        if (stream.is_open()) 
        {
            while(stream.peek() != EOF)
                str << (char) stream.get();
            stream.close();
            return str.str().c_str();
        }
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

    void InitiateStaticHandler(std::string config_file) {
        parser_.Parse(config_file.c_str(), &out_config_);
        handler_ = StaticHandler::create(out_config_, "/usr/src/project");
    }

    std::unique_ptr<Request> make_request(std::string raw_req)
    {
        return Request::request_handler(raw_req);
    }
};


TEST_F(StaticHandlerTest, JpgRequest) {
    std::unique_ptr<Request> req = make_request("GET /static2/photo1.jpg HTTP/1.1\r\n\r\n");
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, JpegRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/jpeg_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, PngRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/png_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, HtmRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/htm_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, HtmlRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/html_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, GifRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/gif_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, InvalidRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/invalid_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND); 
}

TEST_F(StaticHandlerTest, PdfRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/pdf_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, TiffRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/pdf_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, DocRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/doc_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, TxtRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/txt_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, CsvRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/csv_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, CssRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/css_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, jsRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/js_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, tifRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/tif_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, zipRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/zip_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, rtfRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/rtf_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, jsonRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/json_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, xmlRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/xml_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::OK); 
}

TEST_F(StaticHandlerTest, excelRequest) {
    std::unique_ptr<Request> req = make_request(get_req_string("../tests/static_handler_tests/excel_request"));
    InitiateStaticHandler("../tests/configs/static_config");
    std::unique_ptr<Response> resp = handler_->HandleRequest(*(req.get()));
    EXPECT_EQ(resp->getStatusCode(), Response::NOT_FOUND); 
}