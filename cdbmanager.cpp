#include "cdbmanager.h"

#include "tools.h"

#include <utility>
#include <fstream>
#include <vector>
#include <iostream>

#include <string.h>

using pairResult = std::pair<bool, std::string>;


CDBManager::CDBManager(const size_t nThreadCount/*=4*/, const size_t nConnCount/*=10*/):m_threadPool(nThreadCount), m_connPool(nConnCount)
{}

//submit a task into thread pool, return related future object
template<typename Func, typename... Args>
auto CDBManager::submit(Func && func, Args&&... args)->std::future<decltype(func(args...))>
{
    return m_threadPool.addTask(func, args...);
}

//performing the db query operation, return error message and empty query result on some errors occuring, or query result and empty error message
CDBManager::optResult CDBManager::query(const std::string & strSQL)
{
    auto query_lambda = [this, strSQL]()->std::pair<std::string, query_result>{
        if(strSQL.empty())
            return {std::string("empty sql statement..."), query_result()};

        auto & pConn = this->m_connPool.getAConn();
        CDBConnectPool::ConnManager connManger(pConn);

        //std::cout << "thread_id:" << std::this_thread::get_id() << "  conn_address:" << pConn.second.get() << std::endl;

        //perform db query
        if (mysql_query(pConn.second.get(), strSQL.c_str())) {
            return {std::string(mysql_error(pConn.second.get())), query_result()};
        }

        //get the result
        std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> pRes(mysql_store_result(pConn.second.get()), &mysql_free_result);
        if (nullptr == pRes) {
            return {std::string(mysql_error(pConn.second.get())), query_result()};
        }

        //get the result of the query
        MYSQL_FIELD *fields = mysql_fetch_fields(pRes.get());
        int nFiledCount = mysql_num_fields(pRes.get());
        MYSQL_ROW row = nullptr;

        int nRowCount = 0;
        query_result mpResult;
        while ((row = mysql_fetch_row(pRes.get()))) {
            hash_map<std::string, std::string> mpFiledValue;//a row, key->field name, value->field value

            for (int i = 0; i < nFiledCount; i++) {
                mpFiledValue.emplace(std::string(fields[i].name), std::string(row[i] ? row[i] : _NO_VALUE_));
            }

            mpResult.emplace(nRowCount++, std::move(mpFiledValue));
        }

        return std::make_pair(std::string(), std::move(mpResult));
    };

    return this->submit(std::move(query_lambda));
}

//performing db update operation, such update, add, insert, return true and empty error message on sccuess, or fale and related error message
CDBManager::updateResult CDBManager::update(const std::string & strSQL)
{
    auto update_lambda = [this, strSQL]()->pairResult{
        if(strSQL.empty()){
            return {false, std::string("empty sql statement...")};
        }

        auto & pConn = this->m_connPool.getAConn();
        CDBConnectPool::ConnManager connManger(pConn);

        //perform db query
        if (mysql_query(pConn.second.get(), strSQL.c_str())) {
            return {false, std::string(mysql_error(pConn.second.get()))};
        }else{
            return {true, {}};
        }
    };

    return this->submit(std::move(update_lambda));
}


//performing db binary data update operation in accordance with the given table name, field name, condition statement and file name, meaning set the given file name as the given field value in given table name, return true and empty error message, or false and related error message
CDBManager::updateResult CDBManager::updateBin( const std::string & strTableName,
                                                const std::string & strFieldName,
                                                const std::string & strFileName,
                                                const std::string & strWhere)
{
    auto updateBin = [strTableName, strFieldName, strFileName, strWhere, this]()->pairResult{
        auto & pConn = this->m_connPool.getAConn();
        CDBConnectPool::ConnManager connManager(pConn);

        std::ifstream file(strFileName.c_str(), std::ios::binary);//RAII file stream
        if(!file.is_open()){
            return {false, Tools::format("failed to open file:{}", strFileName)};
        }

        std::vector<char> imageData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::unique_ptr<MYSQL_STMT, decltype(&mysql_stmt_close)>pStmt(nullptr, &mysql_stmt_close);
        pStmt.reset(mysql_stmt_init(pConn.second.get()));
        if (!pStmt) {
            return {false, mysql_error(pConn.second.get())};
        }

        std::string && strSQL = Tools::format("UPDATE {} SET {} = ? WHERE {}", strTableName, strFieldName, strWhere);
        if (mysql_stmt_prepare(pStmt.get(), strSQL.c_str(), strSQL.size()) != 0) {
            return {false, mysql_error(pConn.second.get())};
        }

        MYSQL_BIND bind[1] = {0};
        unsigned long nLength = imageData.size();
        bind[0].buffer_type = MYSQL_TYPE_LONG_BLOB;
        bind[0].buffer = (char*)imageData.data();
        bind[0].is_null = 0;
        bind[0].length = &nLength;//length to set
        if (mysql_stmt_bind_param(pStmt.get(), bind)) {
            return {false, mysql_error(pConn.second.get())};
        }

        //performing update statement
        if (mysql_stmt_execute(pStmt.get())) {
            return {false, mysql_error(pConn.second.get())};
        }

        return {true, {}};
    };

    return this->submit(std::move(updateBin));
}


//execute the given sql statement, return false on some error occurring and related error message, or true
CDBManager::execResult CDBManager::execute(const std::string & strSQL)
{
    auto exec_lambda = [strSQL, this]()->pairResult{
        auto & pConn = this->m_connPool.getAConn();
        CDBConnectPool::ConnManager connManager(pConn);

        //perform db query
        if (mysql_query(pConn.second.get(), strSQL.c_str())) {
            return {false, std::string(mysql_error(pConn.second.get()))};
        }else{
            return {true, {}};
        }
    };

    return this->submit(std::move(exec_lambda));
}


//performing db binary data query operation in accordance with the given table name, field name and condition statement, return -1(unsigned, meaning 0xFFFFFFFFFFFFFFFF) and empty pointer on failure, or binary data length and related data pointer
CDBManager::binResult CDBManager::binQuery(const std::string & strTableName,
                                           const std::string & strFieldName,
                                           const std::string & strWhere)
{
    auto binQuery_lambda = [strTableName, strFieldName, strWhere, this]()->std::pair<std::uint64_t, std::shared_ptr<char[]>>{
        auto & pConn = this->m_connPool.getAConn();
        CDBConnectPool::ConnManager connManager(pConn);

        auto && strSQL = Tools::format("SELECT {} FROM {} WHERE {}", strFieldName, strTableName, strWhere);
        if (mysql_query(pConn.second.get(), strSQL.c_str())) {
            return {-1, {}};
        }

        std::unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> pRes(nullptr, &mysql_free_result);
        pRes.reset(mysql_store_result(pConn.second.get()));

        MYSQL_ROW curRow = nullptr;
        while((curRow = mysql_fetch_row(pRes.get()))){
            auto pSize = mysql_fetch_lengths(pRes.get());
            std::shared_ptr<char []> pBinData(new char[pSize[0]]);
            std::memset(pBinData.get(), 0x00, pSize[0]);
            std::memcpy(pBinData.get(), curRow[0], pSize[0]);

            return {*pSize, pBinData};
        }

        return {-1, {}};
    };

    return this->submit(std::move(binQuery_lambda));
}




