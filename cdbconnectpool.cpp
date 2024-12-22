#include "cdbconnectpool.h"

#include "SysConfig.h"

#include <stdexcept>

CDBConnectPool::CDBConnectPool(const size_t nConnCount)
{
    if(nConnCount >= 0)
        this->m_nConnCount = nConnCount;
    else
        this->m_nConnCount = 1;

    //creat db connection in accordance with nConnCount
    this->connect2DB();
}

//creat the given db connection, and adding them into m_lstConns
void CDBConnectPool::connect2DB()
{
    int nTryTime = 3;
    for(size_t ii = 0; ii < this->m_nConnCount; ii++){
        DBConnPtr pConnTemp(mysql_init(NULL), &mysql_close);
        if(!pConnTemp){
            if(0 == --nTryTime)
                throw std::runtime_error("failed to calling 'mysql_init'...");
            continue;
        }

        auto pRet = mysql_real_connect(pConnTemp.get(),
                                       DBparams.strIp.c_str(),
                                       DBparams.strUsername.c_str(),
                                       DBparams.strPassword.c_str(),
                                       DBparams.strDBName.c_str(),
                                       DBparams.nPort, NULL, 0);

        if(nullptr != pRet){
            m_lstConns.emplace_back(false, std::move(pConnTemp));
        }else{
            this->m_strErrMsg = std::string(mysql_error(pConnTemp.get()));
            if(0 == --nTryTime)
                throw std::runtime_error(this->m_strErrMsg);
        }
    }
}

//return related error message when creating connection
const std::string & CDBConnectPool::getErrMsg() const
{
    return this->m_strErrMsg;
}

//return the db connection count in m_lstConns
int CDBConnectPool::getConnCount() const
{
    return this->m_nConnCount;
}

//return a free db connection from m_lstConns, and it would creat a new db connection when all conneciton being busy, the returned value should be managed by class ConnManager
//the first item marking whether the connection being or not, true for busy, false for free
std::pair<std::atomic<bool>, CDBConnectPool::DBConnPtr> & CDBConnectPool::getAConn()
{
    {
        std::lock_guard<std::mutex> lock_guard(this->m_mtx);
        for(auto & conn : m_lstConns){
            if(false == conn.first.load()){
                conn.first.store(true);
                return conn;
            }
        }
    }

    //if all conn being busy, and creat a new connection
    int nTryTime = 3;
    for(size_t ii = 0; ii < 3; ii++){
        DBConnPtr pConnTemp(mysql_init(NULL), &mysql_close);
        if(!pConnTemp){
            if(0 == --nTryTime)
                throw std::runtime_error("failed to calling 'mysql_init'...");
            continue;
        }

        auto pRet = mysql_real_connect(pConnTemp.get(),
                                       DBparams.strIp.c_str(),
                                       DBparams.strUsername.c_str(),
                                       DBparams.strPassword.c_str(),
                                       DBparams.strDBName.c_str(),
                                       DBparams.nPort, NULL, 0);
        if(nullptr != pRet){
            std::lock_guard<std::mutex> lock_guard(this->m_mtx);
            m_lstConns.emplace_back(true, std::move(pConnTemp));//making flag busy
            m_nConnCount += 1;
            return m_lstConns.back();
        }else{
            this->m_strErrMsg = std::string(mysql_error(pConnTemp.get()));
            if(0 == --nTryTime)
                throw std::runtime_error(this->m_strErrMsg);
        }
    }
}
