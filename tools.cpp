#include "tools.h"

#include <cmath>

namespace Tools {

//split the strSrc in accordance with the strSplit
std::vector<std::string> split(const std::string& strSrc, const std::string& strSplit) {
    std::vector<std::string> result;

    if (strSplit.empty()) {
        return {strSrc};
    }

    size_t start = 0;
    size_t end = strSrc.find(strSplit);

    while (end != std::string::npos) {
        std::string token = strSrc.substr(start, end - start);

        if (!token.empty() && token.find_first_not_of(" \t\n\r\f\v") != std::string::npos) {
            result.push_back(token);
        }

        start = end + strSplit.length();
        end = strSrc.find(strSplit, start);
    }

    std::string token = strSrc.substr(start);
    if (!token.empty() && token.find_first_not_of(" \t\n\r\f\v") != std::string::npos) {
        result.push_back(token);
    }

    return result;
}

//return true on a being equal to b, or false
bool fEqual(double a, double b /*= 0.0*/)
{
    double fEpli = 0.000001;

    if(std::fabs(a - b) <= fEpli){
        return true;
    }

    return false;
}

} // namespace Tools
