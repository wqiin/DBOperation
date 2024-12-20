
#include "threadPool.hpp"
#include "cmysql.h"

#include <iostream>
#include <chrono>

int main(int argc, char *argv[])
{
    StDBParams params;
    params.strPassword = "shan53...";
    params.strDBName = "wqiin";
    const std::string strSQL = "select * from course";

    auto test_lambda = [](){
        CMySQL sql;
        std::vector<StCourse> vecResult;
        sql.query_table(vecResult);
    };


    UT::CThreadPool pool(4);
    std::vector<std::future<void>> vecRet;

    auto start = std::chrono::high_resolution_clock::now();
    for(int ii = 0; ii< 10000; ii++){
        vecRet.emplace_back(pool.addTask(test_lambda));
    }

    for(auto & item : vecRet){
        item.get();
    }

    // 获取当前时间点，表示代码结束执行的时刻
    auto end = std::chrono::high_resolution_clock::now();

    // 计算执行时长
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 输出执行时长
    std::cout << "Code execution time: " << duration.count() << " milliseconds" << std::endl;



    return 0;
}
