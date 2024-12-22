#include "cmysql.h"

#include "cdbmanager.h"
#include "tools.h"
#include "SysConfig.h"

bool CMySQL::query_course(std::vector<StCourse> & vecResult)
{
    vecResult.clear();
    const std::string strSQL = "select * from course";

    //wait the query ending with structure binding
    auto && [strErrMsg, mpResults] = DBOPT.query(strSQL).get();
    if(!strErrMsg.empty()){
        std::cout << "Db operation error message:"  << strErrMsg << std::endl;
        return false;
    }

    //access the result of such the query
    for(size_t ii = 0; ii < mpResults.size(); ii++){
        StCourse stTemp;
        stTemp.nID = mpResults.getItem<std::int64_t>(ii, "id");
        stTemp.strCourseName = mpResults.getItem<std::string>(ii, std::string("name"));
        stTemp.nRelatedID = mpResults.getItem<std::uint64_t>(ii, std::string("t_id"));
        stTemp.fFloat = mpResults.getItem<float>(ii, "float");
        stTemp.fDouble = mpResults.getItem<double>(ii, "double");
        stTemp.nDecimal = mpResults.getItem<int>(ii, "decimal");
        stTemp.time = mpResults.getItem<StTime>(ii, "time");
        stTemp.date = mpResults.getItem<StDate>(ii, "date");
        stTemp.datetime = mpResults.getItem<StDateTime>(ii, "datetime");
        stTemp.point = mpResults.getItem<StPoint<int>>(ii, "point");
        stTemp.rect = mpResults.getItem<StRect<int>>(ii, "rectangle");

        vecResult.emplace_back(stTemp);
    }

    return true;
}

bool CMySQL::update_course(const std::string & strSQL)
{
    auto && [bIsOK, strErrMsg] = DBOPT.update(strSQL).get();
    if(!bIsOK){
        std::cout << "update error message:" << strErrMsg << std::endl;
        return false;
    }

    return true;
}

bool CMySQL::updateBin()
{
    auto [bIsOk, strErrMsg] = DBOPT.updateBin("girls", "pic", "/Users/wqiin/Desktop/pic.jpg", "id = 5").get();

    if(!bIsOk)
        std::cout << "error message: " << strErrMsg << std::endl;;

    return bIsOk;
}


//check whether the given field existing or not in the given table, return true on existing or false
bool CMySQL::checkFieldExist(const std::string & strTableName, const std::string & strFieldName)
{
    const std::string strSQL = Tools::format("SELECT COUNT(*) as isExist FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '{}' AND TABLE_NAME = '{}' AND COLUMN_NAME = '{}'", DBparams.strDBName, strTableName, strFieldName);

    auto && [strErrMsg, mpResults] = DBOPT.query(strSQL).get();
    if(!strErrMsg.empty()){
        std::cout << "Db operation error message:"  << strErrMsg << std::endl;
        return false;
    }

    return mpResults.getItem<int>(0, "isExist");
}


//execute the given sql statement,return true on executuion success, or false
bool CMySQL::execSQL(const std::string & strSQL)
{
    auto && [bIsOk, strErrMsg] = DBOPT.execute(strSQL).get();
    if(!bIsOk){
        std::cout << "error messge of execute:" << strErrMsg << std::endl;
        return false;
    }

    return true;
}

//check the given field existing or not, and create it if not existing with the given sql statement as the third parameter
//return true on creation success or existing already, or falses
bool CMySQL::creatFieldIfNotExist(const std::string & strTableName, const std::string & strFieldName, const std::string & strSQL)
{
    if(!checkFieldExist(strTableName, strFieldName)){
        return execSQL(strSQL);
    }

    return true;
}
