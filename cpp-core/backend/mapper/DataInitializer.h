//
// Created by opus arc on 2026/1/31.
//

#ifndef BIBO_NO_AOZORA_DATAINITIALIZER_H
#define BIBO_NO_AOZORA_DATAINITIALIZER_H
#include "duckdb.hpp"

#define DATABASE_PATH "../backend/database/databaseDB.duckdb"

#define COURSE_COMPLIER_PATH "../backend/database/generated/courseComplier.csv"


class DataInitializer{
public:

    // 连接 duckdb 数据库
    duckdb::DuckDB databaseDB_2{DATABASE_PATH};
    duckdb::Connection databaseDB_2_con{databaseDB_2};

    DataInitializer();
    ~DataInitializer();

    // 初次编译 course 表， 算出重要的启动数据
    void courseComplier();

    // 初次检查 user 表，合并 courseComplier 的数据
    void learningDataInitializer();

    // 导出所有初始化成功的数据库表
    bool export_all_tables_to_csvs();


    // 必须要的工具函数 从数据库检查中引入的
    bool export_table_to_csv(const std::string &table_name, const std::string &csv_path);
};


#endif //BIBO_NO_AOZORA_DATAINITIALIZER_H
