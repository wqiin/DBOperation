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


template<typename Func, typename... Args>
auto CDBManager::submit(Func && func, Args&&... args)->std::future<decltype(func(args...))>
{
    return m_threadPool.addTask(func, args...);
}

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




bool CDBManager::binQuery()
{
    {
        auto & pConn = this->m_connPool.getAConn();
        const char *query = "SELECT image FROM course WHERE id = 53";

        mysql_query(pConn.second.get(), query);

        MYSQL_RES * res = mysql_store_result(pConn.second.get());
        //int nRoe = mysql_affected_rows(pConn.second.get());

        MYSQL_ROW curRow = nullptr;
        unsigned long * pSize = nullptr;
        char * pTemp = nullptr;

        while((curRow = mysql_fetch_row(res))){

            pSize = mysql_fetch_lengths(res);

            pTemp = new char[pSize[0]];
            int nSize = sizeof(pSize[0]);
            memset(pTemp, 0x00, nSize);

            memcpy(pTemp, curRow[0], pSize[0]);
            break;
        }

        // 将图片保存为文件（例如 pic.jpg）
        std::ofstream outFile("/Users/wqiin/Desktop/fetched_pic.jpg", std::ios::binary);
        if (!outFile) {
            return EXIT_FAILURE;
        }

        outFile.write(pTemp, pSize[0]);  // 写入图片数据
        outFile.close();
        //cout << "Image successfully saved to file." << endl;

        mysql_free_result(res);

        return EXIT_SUCCESS;
    }
}



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
