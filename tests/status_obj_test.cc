#include "status_obj.h"
#include "gtest/gtest.h"
#include "gtest/gtest_prod.h"
#include "gmock/gmock.h"
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <stdexcept>
#include <map>

class StatusObjTest : public ::testing::Test 
{
    protected:
    StatusObject test; 

};

TEST_F(StatusObjTest, AddEntryTest)
{
    StatusObject::addStatusEntry("this","one");
    std::vector<std::tuple<int,std::string,std::string>> status_entries = StatusObject::getStatusEntries();
    bool correct = false;
    for (std::vector<std::tuple<int,std::string,std::string>>::const_iterator it = status_entries.begin();
         it != status_entries.end(); it++)
    {
        if ((std::get<1>(*it) == "this") && (std::get<2>(*it) == "one") && (std::to_string(std::get<0>(*it)) == "1"))
        {
                correct = true;
        }
    }
    EXPECT_TRUE(correct);
}

TEST_F(StatusObjTest, AddTwoEntryTest)
{
    StatusObject test; 
    StatusObject::addStatusEntry("this","one");
    StatusObject::addStatusEntry("this","one");
    std::vector<std::tuple<int,std::string,std::string>> status_entries = StatusObject::getStatusEntries();
    bool correct = false;
    for (std::vector<std::tuple<int,std::string,std::string>>::const_iterator it = status_entries.begin();
         it != status_entries.end(); it++)
    {
        if ((std::get<1>(*it) == "this") && (std::get<2>(*it) == "one") && (std::to_string(std::get<0>(*it)) == "2"))
        {
                correct = true;
        }
    }
    EXPECT_TRUE(correct);
}

TEST_F(StatusObjTest, TwoStatusCallTest)
{
    StatusObject test; 
    StatusObject::addStatusEntry("this","one");
    StatusObject::addStatusEntry("this","one");
    int totalcalls = StatusObject::getStatusCalls();
    EXPECT_TRUE(totalcalls = 2);
}