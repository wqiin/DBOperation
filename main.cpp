
#include "cmysql.h"
#include "cdbmanager.h""

#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
    void(argc),void(argv);

    auto test_lambda = [](){
        CMySQL sql;
        std::vector<StCourse> vecResult;
        sql.query_course(vecResult);
    };

/*
    UT::CThreadPool pool(4);
    std::vector<std::future<void>> vecRet;

    auto start = std::chrono::high_resolution_clock::now();
    for(int ii = 0; ii< 10000; ii++){
        vecRet.emplace_back(pool.addTask(test_lambda));
    }

    for(auto & item : vecRet){
        item.get();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Code execution time: " << duration.count() << " milliseconds" << std::endl;

    std::cout << "copy times:" << query_result::nCopyTimes << std::endl;
    std::cout << "move times:" << query_result::nMoveTime << std::endl;
    */

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

    return 0;
}
