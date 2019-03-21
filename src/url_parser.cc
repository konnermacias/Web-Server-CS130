#include "url_parser.h"
#include <boost/algorithm/string/replace.hpp>
      
// Function referenced from http://www.cplusplus.com/forum/general/94849/
std::string URLParser::urlDecode(std::string eString) 
{
    std::string ret;
    char ch;
    int i, j;
    for (i=0; i<eString.length(); i++) 
    {
        if (int(eString[i])==37) 
        {
            sscanf(eString.substr(i+1,2).c_str(), "%x", &j);
            ch=static_cast<char>(j);
            ret+=ch;
            i=i+2;
        } else
        {
            ret+=eString[i];
        }
    }
    return (ret);
}

std::string URLParser::htmlEncode(std::string data) 
{
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            case ' ':  buffer.append("&nbsp;");      break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}

std::string URLParser::htmlDecode(std::string data) 
{
    std::string buffer = data;

    boost::replace_all(buffer, "&amp;", "&");
    boost::replace_all(buffer, "&quot;", "\"");
    boost::replace_all(buffer, "&apos;", "\'");
    boost::replace_all(buffer, "&lt;", "<");
    boost::replace_all(buffer, "&gt;", ">");
    boost::replace_all(buffer, "&nbsp;", " ");

    return buffer;
}