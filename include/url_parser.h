#pragma once

#include <iostream>
#include <string>

class URLParser 
{
    public:
        static std::string urlDecode(std::string eString);
        static std::string htmlEncode(std::string data);
        static std::string htmlDecode(std::string data);
};