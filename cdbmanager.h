#ifndef CDBMANAGER_H
#define CDBMANAGER_H

#include "threadPool.hpp"
#include "cdbconnectpool.h"

#include <optional>
#include <sstream>

template<typename key, typename value>
using hash_map = std::unordered_map<key, value>;
using map_result = hash_map<std::uint64_t, hash_map<std::string, std::string>>;

class query_result : public map_result{
public:
    inline static std::atomic<int> nCopyTimes = 0;
    inline static std::atomic<int> nMoveTime = 0;

public:
    //default constructor
    query_result() :map_result(){}

    //move constructor
    query_result(query_result && other) noexcept{
        this->swap(other);
        nMoveTime += 1;
    }


    //get the value in No.nIndex row and filed name being strFieldName, otherwise throw relative expection with error message
    template<class T>
    T getItem(const size_t nIndex, const std::string & strFieldName)
    {
        if(!this->count(nIndex))
            throw std::out_of_range("invalid index access or no such the target row in query result:[" + std::to_string(nIndex) + "]");

        if(strFieldName.empty())
            throw std::runtime_error("empty field name");

        if(!this->at(nIndex).count(strFieldName))
            throw std::runtime_error("invalid filed name access or no such the field in query result:[" + strFieldName + std::string("]"));

        T retValue;
        const std::string & strValue = this->at(nIndex).at(strFieldName);
        std::istringstream stream(strValue);
        if (!(stream >> retValue)) {
            throw std::invalid_argument("invalid conversion from string to " + std::string(typeid(T).name()));
        }

        return retValue;
    }

    // left value assign operator override
    query_result & operator= (const query_result & other) {
        if (this == &other) {
            return *this;
        }

        //using the base class(map_result) assign operator instead
        nCopyTimes += 1;
        static_cast<map_result&>(*this) = static_cast<const map_result&>(other);
        return *this;
    }

    //right value assign operator override
    query_result & operator= (const query_result && other) {
        if (this == &other) {  // 检查自赋值
            return *this;
        }

        //using the base class(map_result) assign operator instead
        nCopyTimes += 1;
        static_cast<map_result&>(*this) = static_cast<const map_result&>(other);
        return *this;
    }
};


class CDBManager
{
public:
    static CDBManager & getInst(){
        static CDBManager inst(4, 8);
        return inst;
    }

    //db query operation interface
    using optResult = std::future<std::pair<std::string, query_result>>;
    optResult query(const std::string & strSQL);

    //add, delete and update operation interface
    using updateResult = std::future<std::pair<bool, std::string>>;
    updateResult update(const std::string & strSQL);

private:
    explicit CDBManager(const size_t nThreadCount = 4, const size_t nConnCount = 10);

    //sumbit the some db operation task into the thread pool
    template<typename Func, typename... Args>
    auto submit(Func && func, Args&&... args)->std::future<decltype(func(args...))>;

private:
    UT::CThreadPool m_threadPool;
    CDBConnectPool m_connPool;
};

#define DBOPT CDBManager::getInst()

#endif // CDBMANAGER_H
