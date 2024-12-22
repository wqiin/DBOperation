#ifndef TOOLS_H
#define TOOLS_H

#include <string>
#include <ostream>
#include <sstream>
#include <vector>
#include <chrono>

namespace Tools {

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


std::string format(const std::string& fmt);

template <typename T>
std::string format(const std::string& fmt, T&& arg);

template <typename T, typename... Args>
std::string format(const std::string& fmt, T&& arg, Args&&... args);

std::vector<std::string> split(const std::string& strSrc, const std::string& strSplit);


template <typename T>
std::string format(const std::string& fmt, T&& arg) {
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
std::string format(const std::string& fmt, T&& arg, Args&&... args) {
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

} // namespace Tools

#endif // TOOLS_H
