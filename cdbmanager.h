#ifndef CDBMANAGER_H
#define CDBMANAGER_H

#include "threadPool.hpp"
#include "cdbconnectpool.h"

#include <sstream>

template<typename key, typename value>
using hash_map = std::unordered_map<key, value>;
using map_result = hash_map<std::uint64_t, hash_map<std::string, std::string>>;

//empty value when some field in db query result being empty
#define _NO_VALUE_ ""

//such the class being designed to manage the results from db query
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

        //no such the field name in query result
        if(!this->at(nIndex).count(strFieldName)){
            return T();
        }

        const std::string & strValue = this->at(nIndex).at(strFieldName);
        if(std::string(_NO_VALUE_) == strValue)
            return T();

        T retValue;
        std::istringstream stream(strValue);
        if (!(stream >> retValue)) {
            throw std::invalid_argument("invalid conversion from string to " + std::string(typeid(T).name()));
        }

        return retValue;
    }

    // left value assign operator override
    query_result & operator = (const query_result & other) {
        if (this == &other) {
            return *this;
        }

        //using the base class(map_result) assign operator instead
        nCopyTimes += 1;
        static_cast<map_result&>(*this) = static_cast<const map_result&>(other);
        return *this;
    }

    //right value assign operator override
    query_result & operator = (const query_result && other) {
        if (this == &other) {  // 检查自赋值
            return *this;
        }

        //using the base class(map_result) assign operator instead
        nCopyTimes += 1;
        static_cast<map_result&>(*this) = static_cast<const map_result&>(other);
        return *this;
    }
};


//such the class being designed to manage the thread pool and db connection pool
class CDBManager
{
public:
    static CDBManager & getInst(){
        static CDBManager inst(4, 8);//4 threads and 8 db connections
        return inst;
    }

    //performing the db query operation, return error message and empty query result on some errors occuring, or query result and empty error message
    using optResult = std::future<std::pair<std::string, query_result>>;
    optResult query(const std::string & strSQL);


    //performing db update operation, such update, add, insert, return true and empty error message on sccuess, or fale and related error message
    using updateResult = std::future<std::pair<bool, std::string>>;
    updateResult update(const std::string & strSQL);


    //performing db binary data update operation in accordance with the given table name, field name, condition statement and file name, meaning set the given file name as the given field value in given table name, return true and empty error message, or false and related error message
    updateResult updateBin(const std::string & strTableName,
                           const std::string & strFieldName,
                           const std::string & strFileName,
                           const std::string & strWhere);

    //performing db binary data query operation in accordance with the given table name, field name and condition statement, return -1(unsigned, meaning 0xFFFFFFFFFFFFFFFF) and empty pointer on failure, or binary data length and related data pointer
    using binResult = std::future<std::pair<std::uint64_t, std::shared_ptr<char[]>>>;
    binResult binQuery(const std::string & strTableName,
                       const std::string & strFieldName,
                       const std::string & strWhere);

    //execute the given sql statement, return false on some error occurring and related error message, or true and empty error message
    using execResult = std::future<std::pair<bool, std::string>>;
    execResult execute(const std::string & strSQL);

private:
    explicit CDBManager(const size_t nThreadCount = 4, const size_t nConnCount = 10);

    //submit a task into thread pool, return related future object
    template<typename Func, typename... Args>
    auto submit(Func && func, Args&&... args)->std::future<decltype(func(args...))>;

private:
    UT::CThreadPool m_threadPool;
    CDBConnectPool m_connPool;
};

#define DBOPT CDBManager::getInst()

#endif // CDBMANAGER_H
