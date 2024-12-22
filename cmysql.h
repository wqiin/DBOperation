#ifndef CMYSQL_H
#define CMYSQL_H

#include "data_type_defination.h"

// such the class being designed to focus on the basic db operation

class CMySQL
{
public:
    CMySQL() = default;
    ~CMySQL() = default;

    //operating some basic table
    bool query_course(std::vector<StCourse> & vecResult);
    bool update_course(const std::string & strSQL);


    bool updateBin();


    bool checkFiledExist(const std::string & strTableName, const std::string & strFieldName);

    //execute the given sql statement
    bool execSQL(const std::string & strSQL);

    //check the given field existing or not, and create it if not existing with the given sql statement as the third parameter
    bool creatFieldIfNotExist(const std::string & strTableName, const std::string & strFieldName, const std::string & strSQL);
};

#endif // CMYSQL_H
