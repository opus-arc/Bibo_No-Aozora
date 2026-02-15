//
// Created by opus arc on 2026/1/31.
//

#ifndef BIBO_NO_AOZORA_DATAINITIALIZER_H
#define BIBO_NO_AOZORA_DATAINITIALIZER_H
#include <filesystem>

#include "../../utils/external/duckdb/include/duckdb.hpp"

#define DATABASE_PATH "../backend/database/databaseDB.duckdb"

#define COURSE_COMPLIER_PATH "../backend/database/generated/courseComplier.csv"

#define HISTORY_PATH "../backend/database/generated/history/"

#define REVIEWS_PER_DAY 10

// #define GENERATED_FOLDER_PATH "../backend/database/generated/"

#define VIRTUAL_DATE_TEST_SWITCH false

// 只识别 YYMMDD 格式的日期
#define VIRTUAL_DATE "260204"


class DataInitializer{
public:

    // 连接 duckdb 数据库
    duckdb::DuckDB databaseDB_2{DATABASE_PATH};
    duckdb::Connection databaseDB_2_con{databaseDB_2};

    DataInitializer();
    ~DataInitializer();

    // 清空 courseComplier 确保是空的 course 的计算容器
    void clearCourseComplierTable();

    // 初次编译 course 表， 算出重要的启动数据
    void courseComplier();

    // 初次检查 user 表，合并 courseComplier 的数据
    void learningDataInitializer();

    // 导出一张表的工具函数
    bool export_table_to_csv(const std::string &table_name, const std::string &csv_path);

    // 导出所有初始化成功的数据库表
    bool export_all_tables_to_csvs();

    // 检查今日表
    static std::filesystem::path todayCsvChecker();

    // 决定今日表的内容
    void selectTodayCards();

};


#endif //BIBO_NO_AOZORA_DATAINITIALIZER_H
