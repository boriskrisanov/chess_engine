#pragma once

#include <vector>
#include <string>

inline std::vector<std::string> splitString(std::string str, std::string delimiter)
{
    std::vector<std::string> result;

    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        result.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    result.push_back(str);

    return result;
}