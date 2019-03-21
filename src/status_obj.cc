#include "status_obj.h"


void StatusObject::addStatusEntry(std::string request_url, std::string resp_code)
{
   
   std::tuple<int,std::string,std::string> status_entry(1, request_url, resp_code);
   for (unsigned int i = 0; i < status_entries_.size(); i++)
   {
       if ((std::get<1>(status_entries_[i]) == request_url) && (std::get<2>(status_entries_[i])== resp_code))
       {
            (std::get<0>(status_entries_[i])) += 1;
            return;
       }
   }
   status_entries_.push_back(status_entry);
   return;
}

std::vector<std::tuple<int,std::string,std::string>> StatusObject::getStatusEntries()
{
  
   return status_entries_;
}

int StatusObject::getStatusCalls()
{
   unsigned int count = 0;
   for (unsigned int i = 0; i < status_entries_.size(); i++)
   {
      count += (std::get<0>(status_entries_[i]));    
   }

   return count;
}

std::vector<std::tuple<int,std::string,std::string>> StatusObject::status_entries_;

std::map<std::string, std::string> StatusObject::url_handlers_map_;