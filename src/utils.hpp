#pragma once

#include <string>
#include <vector>


inline std::vector<std::string> splitString(std::string str, const std::string &delimiter)
{
    std::vector<std::string> result;

    size_t pos = 0;
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        std::string token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    result.push_back(str);

    return result;
}