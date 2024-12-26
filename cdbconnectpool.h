#ifndef CDBCONNECTPOOL_H
#define CDBCONNECTPOOL_H

#include "mysql.h"

#include <memory>
#include <string>
#include <list>
#include <atomic>
#include <mutex>


//used to distinguish whether the connection busy or not
constexpr bool CONN_FREE = false;   //db connection being busy
constexpr bool CONN_BUSY = true;    //db connection being free and available


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
            m_pairConn.first.store(CONN_FREE);
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

    //return a free db connection from m_lstConns, and it would creat a new db connection when all conneciton being busy, the returned value should be managed by class ConnManager
    //the first item marking whether the connection being or not, true for busy, false for free
    std::pair<std::atomic<bool>, DBConnPtr> & getAConn();

    //return related error message when creating connection
    const std::string & getErrMsg() const;

    //return the db connection count in m_lstConns
    int getConnCount() const;

private:
    //creat the given db connection, and adding them into m_lstConns
    void connect2DB();

private:
    //key marking wherther the conn being busy
    std::list<std::pair<std::atomic<bool>, DBConnPtr>> m_lstConns;

    //to lock the m_lstConns when appending item backward
    std::mutex m_mtx;

    std::string m_strErrMsg;
    size_t m_nConnCount = 0;
};

#endif // CDBCONNECTPOOL_H
