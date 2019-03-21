cmake_minimum_required(VERSION 3.10.0)
project(git-r-done)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds if we are building in a devel shell
if (CMAKE_BUILD_TYPE STREQUAL "" AND "$ENV{DEVEL_SHELL}" STREQUAL "1")
    message(STATUS "Setting debug build type by default in devel shell")
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest
include(GoogleTest)
add_subdirectory(/usr/src/googletest googletest)

# Enable Boost
# Use static libraries so binaries can be deployed without a full boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Update name and srcs
# add_library(example_lib src/example_lib.cc)

add_library(request src/request.cc)
add_library(server src/server.cc)
add_library(session src/session.cc)
add_library(config_parser src/config_parser.cc)
add_library(response src/response.cc)
add_library(echo_handler src/echo_handler.cc)
add_library(error_handler src/error_handler.cc)
add_library(static_handler src/static_handler.cc)
add_library(status_handler src/status_handler.cc)
add_library(proxy_handler src/proxy_handler.cc)
add_library(dispatcher src/dispatcher.cc)
add_library(handler_manager src/handler_manager.cc)
add_library(status_obj src/status_obj.cc)
add_library(meme_handler src/meme_handler.cc)
add_library(url_parser src/url_parser.cc)
add_library(health_handler src/health_handler.cc)
add_library(bad_handler src/bad_handler.cc)

# Update executable name, srcs, and deps
# add_executable(example_server src/example_server_main.cc)
# target_link_libraries(example_server example_lib Boost::system)

add_executable(server_main src/server_main.cc)
target_link_libraries(server_main server session dispatcher handler_manager echo_handler error_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler request config_parser response status_obj Boost::system Boost::log_setup Boost::log)

# Update test executable name, srcs, and deps
# add_executable(example_lib_test tests/example_lib_test.cc)
# target_link_libraries(example_lib_test example_lib gtest_main)

add_executable(config_parser_test tests/config_parser_test.cc)
target_link_libraries(config_parser_test config_parser status_obj gtest_main Boost::log_setup Boost::log)

add_executable(dispatcher_test tests/dispatcher_test.cc)
target_link_libraries(dispatcher_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(echo_handler_test tests/echo_handler_test.cc)
target_link_libraries(echo_handler_test server session request config_parser status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(error_handler_test tests/error_handler_test.cc)
target_link_libraries(error_handler_test server session dispatcher handler_manager request config_parser error_handler echo_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler response status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(handler_manager_test tests/handler_manager_test.cc)
target_link_libraries(handler_manager_test server session dispatcher handler_manager echo_handler error_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler request config_parser response status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(request_test tests/request_test.cc)
target_link_libraries(request_test server session request config_parser response gtest_main Boost::log_setup Boost::log)

add_executable(response_test tests/response_test.cc)
target_link_libraries(response_test server session request config_parser response gtest_main Boost::log_setup Boost::log)

add_executable(server_test tests/server_test.cc)
target_link_libraries(server_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(session_test tests/session_test.cc)
target_link_libraries(session_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(static_handler_test tests/static_handler_test.cc)
target_link_libraries(static_handler_test server session dispatcher handler_manager request config_parser error_handler echo_handler static_handler status_handler proxy_handler meme_handler url_parser health_handler bad_handler response status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(status_handler_test tests/status_handler_test.cc)
target_link_libraries(status_handler_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(status_obj_test tests/status_obj_test.cc)
target_link_libraries(status_obj_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(proxy_handler_test tests/proxy_handler_test.cc)
target_link_libraries(proxy_handler_test proxy_handler request config_parser response status_obj gtest_main gmock_main Boost::system Boost::log_setup Boost::log)

add_executable(meme_handler_test tests/meme_handler_test.cc)
target_link_libraries(meme_handler_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(health_handler_test tests/health_handler_test.cc)
target_link_libraries(health_handler_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_executable(bad_handler_test tests/bad_handler_test.cc)
target_link_libraries(bad_handler_test server session dispatcher handler_manager request config_parser response error_handler echo_handler static_handler status_handler meme_handler url_parser health_handler bad_handler status_obj gtest_main Boost::system Boost::log_setup Boost::log)

add_library(sqlite3 STATIC IMPORTED)
set_target_properties(sqlite3
  PROPERTIES
  IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libsqlite3.a
  INTERFACE_INCLUDE_DIRECTORIES /usr/include
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_target_properties(sqlite3
    PROPERTIES
      INTERFACE_LINK_LIBRARIES "dl;pthread"
   )
endif()

target_link_libraries (server_main sqlite3)
target_link_libraries (config_parser_test sqlite3)
target_link_libraries (dispatcher_test sqlite3)
target_link_libraries (echo_handler_test sqlite3)
target_link_libraries (error_handler_test sqlite3)
target_link_libraries (handler_manager_test sqlite3)
target_link_libraries (request_test sqlite3)
target_link_libraries (response_test sqlite3)
target_link_libraries (server_test sqlite3)
target_link_libraries (session_test sqlite3)
target_link_libraries (static_handler_test sqlite3)
target_link_libraries (status_handler_test sqlite3)
target_link_libraries (status_obj_test sqlite3)
target_link_libraries (proxy_handler_test sqlite3)
target_link_libraries (meme_handler_test sqlite3)
target_link_libraries (health_handler_test sqlite3)
target_link_libraries (bad_handler_test sqlite3)



# Update with test binary
# gtest_discover_tests(example_lib_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
gtest_discover_tests(config_parser_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(dispatcher_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(echo_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(error_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(handler_manager_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(request_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(response_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(server_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(session_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(static_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(status_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(status_obj_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(proxy_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(meme_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(health_handler_test /usr/src/projects/git-r-done build/googletest)
gtest_discover_tests(bad_handler_test /usr/src/projects/git-r-done build/googletest)

# Update with target/test targets
# include(cmake/CodeCoverageReportConfig.cmake)
#generate_coverage_report(TARGETS example_server example_lib TESTS example_lib_test)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(TARGETS server_main server session dispatcher handler_manager echo_handler error_handler static_handler status_handler proxy_handler request config_parser response status_obj meme_handler url_parser health_handler bad_handler TESTS config_parser_test dispatcher_test echo_handler_test error_handler_test handler_manager_test request_test response_test server_test session_test static_handler_test status_handler_test status_obj_test proxy_handler_test meme_handler_test health_handler_test bad_handler_test)

#Add integration test to cmake 
add_test(NAME Integration COMMAND bash ../tests/integration_test.sh)
add_test(NAME Multithread COMMAND python ../tests/multithread_test.py)
# TODO (!): Add coolcoders test
#add_test(NAME Integration COMMAND bash integration_test.sh WORKING_DIRECTORY /usr/src/project/tests)