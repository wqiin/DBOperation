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
};

#endif // CMYSQL_H
