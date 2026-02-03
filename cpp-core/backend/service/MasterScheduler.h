//
// Created by opus arc on 2026/2/1.
//

#ifndef BIBO_NO_AOZORA_MASTERSCHEDULER_H
#define BIBO_NO_AOZORA_MASTERSCHEDULER_H
#include "duckdb.hpp"

#define DATABASE_PATH "../backend/database/databaseDB.duckdb"


#define VIRTUAL_DATE 2026-02-10
#define REVIEWS_PER_DAY 10

class MasterScheduler {
    // 连接 duckdb 数据库
    duckdb::DuckDB databaseDB_3{DATABASE_PATH};
    duckdb::Connection databaseDB_3_con{databaseDB_3};

    // 选取


};


#endif //BIBO_NO_AOZORA_MASTERSCHEDULER_H