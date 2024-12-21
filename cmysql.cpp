#include "cmysql.h"

#include "cdbmanager.h"


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
