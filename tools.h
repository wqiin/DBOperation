#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <ostream>
#include <sstream>
#include <vector>


namespace Tools {
//used to measurment the time cost between codes
class timeElapsed {
private:
    std::chrono::high_resolution_clock::time_point start_time;

public:
    timeElapsed() {
        restart();
    }

    void restart() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double timeCost() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return duration.count();
    }
};

//string format function
std::string format(const std::string & fmt);

template <typename T>
std::string format(const std::string & fmt, T && arg);

template <typename T, typename... Args>
std::string format(const std::string & fmt, T && arg, Args &&... args);

//return true on a being equal to b, or false. In default conditon, such the function could be used to
//judge whether a being equal to zero
bool fEqual(double a, double b = 0.0);

//split the strSrc in accordance with the strSplit
std::vector<std::string> split(const std::string & strSrc, const std::string & strSplit);

template <typename T>
std::string format(const std::string & fmt, T && arg) {
    std::ostringstream oss;
    size_t pos = fmt.find("{}");

    if (pos != std::string::npos) {
        oss << fmt.substr(0, pos) << std::forward<T>(arg) << fmt.substr(pos + 2);
    } else {
        oss << fmt;
    }

    return oss.str();
}

template <typename T, typename... Args>
std::string format(const std::string & fmt, T && arg, Args &&... args) {
    size_t pos = fmt.find("{}");

    if (pos != std::string::npos) {
        std::ostringstream oss;
        oss << fmt.substr(0, pos) << std::forward<T>(arg);
        oss << format(fmt.substr(pos + 2), std::forward<Args>(args)...);
        return oss.str();
    } else {
        return fmt;
    }
}

//create the given target directory, return true and empty error message on success, or false and related error message
using errMsg = std::string;
std::pair<bool, errMsg> makeDirectory(const std::string & strDir);

//get the datetime with format like '2023-12-23 10:30:25'
std::string getDateTime();

} // namespace Tools

#endif // TOOLS_H
