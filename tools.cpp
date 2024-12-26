#include "tools.h"

#include <cmath>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace fs = std::filesystem;

namespace Tools {

//split the strSrc in accordance with the strSplit
std::vector<std::string> split(const std::string & strSrc, const std::string & strSplit) {
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

//create the given target directory, return true and empty error message on success, or false and related error message
std::pair<bool, errMsg> makeDirectory(const std::string & strDir)
{
    fs::path parentDir = fs::path(strDir).parent_path();
    if(!fs::exists(parentDir)){
        try{
            bool bRet = fs::create_directories(parentDir);//when parent directory exists or permission denied, return false
            if(!bRet)
                return {false, std::string("unknown error occurring when create directory ...")};
            return {true, std::string()};
        }catch(const std::exception & e){
            return {false, e.what()};
        }
    }

    return {true, std::string()};
}

//get the datetime with format like '2023-12-23 10:30:25'
std::string getDateTime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t now_time_t = system_clock::to_time_t(now);
    std::tm * tm_info = std::localtime(&now_time_t);

    std::ostringstream oss;
    oss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

} // namespace Tools
