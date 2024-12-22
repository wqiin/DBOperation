#ifndef CMYSQL_H
#define CMYSQL_H

#include "data_type_defination.h"

// such the class being designed to focus on the basic db operation

class CMySQL
{
public:
    CMySQL() = default;
    ~CMySQL() = default;

    //operating some basic table here
    bool query_course(std::vector<StCourse> & vecResult);

    bool update_course(const std::string & strSQL);

    bool updateBin();


    //check whether the given field existing or not in the given table, return true on existing or false
    static bool checkFieldExist(const std::string & strTableName, const std::string & strFieldName);

    //execute the given sql statement,return true on executuion success, or false
    static bool execSQL(const std::string & strSQL);

    //check the given field existing or not, and create it if not existing with the given sql statement as the third parameter
    //return true on creation success or existing already, or falses
    static bool creatFieldIfNotExist(const std::string & strTableName, const std::string & strFieldName, const std::string & strSQL);


};

#endif // CMYSQL_H
