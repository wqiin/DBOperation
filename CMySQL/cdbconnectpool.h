#ifndef CDBCONNECTPOOL_H
#define CDBCONNECTPOOL_H

#include "mysql.h"

#include <memory>
#include <string>
#include <list>
#include <atomic>
#include <mutex>

class CDBConnectPool
{
private:
    using DBConnPtr = std::unique_ptr<MYSQL, decltype(&mysql_close)>;

public:
    //manager the DB conn returned by method getAConn
    class ConnManager{
    public:
        ConnManager(std::pair<std::atomic<bool>, DBConnPtr> & pairConn):m_pairConn(pairConn){}

        //making such the conn not busy and available
        ~ConnManager(){
            m_pairConn.first.store(false);
        }

    private:
        std::pair<std::atomic<bool>, DBConnPtr> & m_pairConn;
    };


    explicit CDBConnectPool(const size_t nConnCount);
    ~CDBConnectPool() = default;

    CDBConnectPool(const CDBConnectPool &) = delete;
    CDBConnectPool(const CDBConnectPool &&) = delete;
    CDBConnectPool & operator=(const CDBConnectPool &) = delete;
    CDBConnectPool & operator=(const CDBConnectPool &&) = delete;

    std::pair<std::atomic<bool>, DBConnPtr> & getAConn();

    const std::string & getErrMsg() const;
    int getConnCount() const;

private:
    void connect2DB();

private:
    //key marking wherther the conn being busy
    std::list<std::pair<std::atomic<bool>, DBConnPtr>> m_lstConns;

    //to lock the m_lstConns when appending item backward
    std::mutex m_mtx;

    std::string m_strErrMsg;
    size_t m_nConnCount = 2;
};

#endif // CDBCONNECTPOOL_H
