#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <stdexcept>
#include <map>

class StatusObject
{
  public:
    static void addStatusEntry(std::string request_url, std::string resp_code);
    static std::vector<std::tuple<int,std::string,std::string>> getStatusEntries();
    static int getStatusCalls();
    static std::map<std::string, std::string> url_handlers_map_;
  
  private:
    static std::vector<std::tuple<int,std::string,std::string>> status_entries_;
};