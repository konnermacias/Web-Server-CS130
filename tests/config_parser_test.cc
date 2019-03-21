#include "gtest/gtest.h"
#include "config_parser.h"
#include "status_obj.h"

// define fixture
class NginxConfigParserTest : public ::testing::Test {
  protected:
    NginxConfigParser parser_;
    NginxConfig out_config_;

    bool parseString(std::string config_string) {
      std::stringstream config_stream(config_string);
      return parser_.Parse(&config_stream, &out_config_);
    }

    bool parseFile(std::string config_file) {
      return parser_.Parse(config_file.c_str(), &out_config_);
    }
};

class NginxConfigTest : public ::testing::Test {
  protected:
      NginxConfig config_;
      NginxConfigParser parser_;

      bool parseString(std::string config_string) {
        std::stringstream config_stream(config_string);
        return parser_.Parse(&config_stream, &config_);
      }

      bool parseFile(std::string config_file) {
        return parser_.Parse(config_file.c_str(), &config_);
      }
};

// test given example
TEST_F(NginxConfigParserTest, ExampleConfig) {
  EXPECT_TRUE(parseFile("../tests/configs/example_config"));
}

// test invalid input
TEST_F(NginxConfigParserTest, InvalidInput) {
  // wrong braces
  EXPECT_FALSE(parseString("events { worker_connections 4096;"));
  EXPECT_FALSE(parseString("events worker_connections 4096; }"));
  // wrong quotes
  EXPECT_FALSE(parseString("log_format main \'$remote;"));
  EXPECT_FALSE(parseString("log_format main \"$remote;"));
  // wrong semicolon
  EXPECT_FALSE(parseString("include conf/mime.types"));
  EXPECT_FALSE(parseString("server { listen 80; };"));
  EXPECT_FALSE(parseString("server { listen 80 };"));
}

// test comment
TEST_F(NginxConfigParserTest, CommentTest) {
  EXPECT_TRUE(parseString("user www www; ## Default: nobody"));
  EXPECT_EQ(out_config_.statements_.size(),1) << "Only 1 statement in config";
  EXPECT_TRUE(parseString("server { listen 123; }\r\n## Useless comment"));
}

// test quotes
TEST_F(NginxConfigParserTest, QuoteTest) {
  EXPECT_TRUE(parseString("server { test \"80\"; }"));
}

// test unknown tokens
TEST_F(NginxConfigParserTest, UnknownTokens) {
  EXPECT_FALSE(parseString("server { test 80; @^#%^&@#*&)%&#(@*!@)_(!)#_$&%& ABC}"));
}

// statement size checks
TEST_F(NginxConfigParserTest, StatementSizeTest) {
  EXPECT_TRUE(parseString("worker_processes 5; error_log logs/error.log; pid logs/nginx.pid;"));
  EXPECT_EQ(out_config_.statements_.size(), 3) << "Only 3 statements in config";
}


// testing correct behavior when there is no listen token
TEST_F(NginxConfigParserTest, NoListenTest) {
  EXPECT_TRUE(parseString("server { test 80; }"));
}

// testing correct behavior when there is no block name
TEST_F(NginxConfigParserTest, NoNameTest) {
  EXPECT_FALSE(parseString("{ test 80; }"));
  EXPECT_FALSE(parseString("{{ test 80; }}"));
}

// check correct parsing for nested statement
TEST_F(NginxConfigParserTest, NestedParseCheck) {
  std::string config_string = 
    "events {\n "
    "worker_connections 4096; # default 1024\n "
    "worker_rlimit_profile 8192;\n"
    "}\n";
  EXPECT_TRUE(parseString(config_string));
  EXPECT_EQ(out_config_.statements_.at(0)->child_block_->statements_.size(), 2);
  EXPECT_EQ(out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(0), "worker_connections");
  EXPECT_EQ(out_config_.statements_.at(0)->child_block_->statements_.at(0)->tokens_.at(1), "4096");
  EXPECT_EQ(out_config_.statements_.at(0)->child_block_->statements_.at(1)->tokens_.at(0), "worker_rlimit_profile");
  EXPECT_EQ(out_config_.statements_.at(0)->child_block_->statements_.at(1)->tokens_.at(1), "8192");
}

// check ToString
TEST_F(NginxConfigParserTest, ToStringTest) {
  NginxConfigStatement stmt;
  stmt.tokens_.push_back("location");
  stmt.tokens_.push_back("/");
  stmt.tokens_.push_back("{");
  stmt.tokens_.push_back("proxy_pass");
  stmt.tokens_.push_back("http://127.0.0.1:8080");
  stmt.tokens_.push_back(";");
  stmt.tokens_.push_back("}");
  EXPECT_EQ(stmt.ToString(0), "location / { proxy_pass http://127.0.0.1:8080 ; };\n");
}



// check big example
TEST_F(NginxConfigParserTest, FullConfig1) {
  EXPECT_TRUE(parseFile("../tests/configs/full_config"));
}

// check on separate large example for depth of validity
TEST_F(NginxConfigParserTest, FullConfig2) {
  EXPECT_TRUE(parseFile("../tests/configs/full_config2"));
}


// check bad example with no port
TEST_F(NginxConfigParserTest, BadConfig) {
  EXPECT_FALSE(parseFile("../tests/configs/bad_config"));

}
// check object construction
TEST_F(NginxConfigTest, ObjectConstruction) {
  NginxConfig config_test;
  NginxConfigParser configparser_test;
  
  EXPECT_TRUE(configparser_test.Parse("../tests/configs/example_config", &config_test));
}

// check valid port number
TEST_F(NginxConfigTest, PortTest1) {
  EXPECT_TRUE(parseFile("../tests/configs/example_config"));
  EXPECT_EQ(config_.GetPort(), 80);
}

TEST_F(NginxConfigTest, PortTest2) {
  EXPECT_TRUE(parseFile("../tests/prac_requests/prac_request"));
  EXPECT_EQ(config_.GetPort(), 0);
}

// check no port specified
TEST_F(NginxConfigTest, PortTest3) {
  EXPECT_TRUE(parseFile("../tests/configs/no_port_config"));
  EXPECT_EQ(config_.GetPort(), 0);
}