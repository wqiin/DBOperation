
#include "cmysql.h"
#include "threadPool.hpp"
#include "tools.h"

#include <iostream>

char * pCrtTable = "CREATE TABLE users ( \
    user_id INT AUTO_INCREMENT PRIMARY KEY, \
        username VARCHAR(100) NOT NULL,       \
        password VARCHAR(255) NOT NULL,             \
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP )";

int main(int argc, char *argv[])
{
    void(argc),void(argv);

    //string format method usage
    std::string strFmt = Tools::format("Hello, {}! Your score is {}.", "Alice", 95);
    std::cout << strFmt << std::endl;

    //string split method usage
    auto && vecSplit = Tools::split("123_456_789_ ", "_");
    for(const auto & item : vecSplit)
        std::cout << item << "--";
    std::cout << "\n";


    auto test_lambda = [](){
        CMySQL sql;
        std::vector<StCourse> vecResult;
        sql.query_course(vecResult);
    };


    UT::CThreadPool pool(4);
    std::vector<std::future<void>> vecRet;
    Tools::timeElapsed tmElapse;
    for(int ii = 0; ii< 10000; ii++){
        vecRet.emplace_back(pool.addTask(test_lambda));
    }

    std::cout << "Code execution time: " << tmElapse.timeCost() << " milliseconds" << std::endl;
    for(auto & item : vecRet){
        item.get();
    }

    std::cout << "Code execution time: " << tmElapse.timeCost() << " milliseconds" << std::endl;

    auto adu = [](){
        const char * pUpdateSQL = "update course set name='this is the name of the course' where id=54";
        const char * pInsertSQL = "INSERT INTO course (id, name, t_id, datetime, date, time,  point, rectangle) VALUES (54, 'Mathematics', 101, '2024-12-17 08:30:00', '2024-12-17', '14:30:00', '(10, 20)', '[(1,25),(23,56)]')";
        const char * pDeleteSQL = "DELETE FROM course WHERE id = 54";

        CMySQL sql;
        bool bRet = sql.update_course(pInsertSQL);
        if(bRet)
            std::cout << "insert sql OK\n";

        bRet = sql.update_course(pUpdateSQL);
        if(bRet)
            std::cout << "update sql OK\n";

        bRet = sql.update_course(pDeleteSQL);
        if(bRet)
            std::cout << "delete sql OK\n";
    };
    adu();


    if(!CMySQL::creatFieldIfNotExist("course", "new_method_field", "ALTER TABLE course ADD COLUMN new_method_field INT")){
        std::cout << "Some errors occured when operiation\n";
    }

    //create a table in accordance with the given sql statement
    bool bRet = CMySQL::execSQL(pCrtTable);
    if(!bRet){
        std::cout << "execute Error\n";
    }else{
        std::cout << "execute ok\n'";
    }
    return 0;
}
