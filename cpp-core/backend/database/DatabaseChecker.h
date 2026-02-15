//
// Created by opus arc on 2026/1/31.
//

#ifndef BIBO_NO_AOZORA_DATABASECHECKER_H
#define BIBO_NO_AOZORA_DATABASECHECKER_H

#define DATABASE_PATH "../backend/database/databaseDB.duckdb"

#define TEMPLATES_FOLDER_PATH "../backend/database/templates"
#define GENERATED_FOLDER_PATH "../backend/database/generated"
#define MANUAL_FOLDER_PATH "../backend/database/manual"

#define COURSE_COMPLIER_PATH "../backend/database/generated/courseComplier.csv"


#include <duckdb.hpp>


/**
 * 扫描 generated 文件夹下的 csv 文件与数据库中的 duckdb 文件
 * 按照 templates 文件夹下的模版进行表头检查与类型检查或在不存在时按照模板覆盖路径
 */
class DatabaseChecker {
public:
    // 连接 duckdb 数据库
    duckdb::DuckDB databaseDB{DATABASE_PATH};
    duckdb::Connection databaseDB_con{databaseDB};

    DatabaseChecker();

    ~DatabaseChecker();

    // 清空数据库
    void clearDatabase();

    // 存在性检查
    static void existChecker();

    // 模版csv文件入库
    std::vector<std::string> templatesPorter();

    // 将数据库中的模版实例化
    void templatesComplier(const std::string &template_table_name, const std::string &suffix = "_Template");

    // 数据库表导出为 csv
    bool export_table_to_csv(const std::string &table_name, const std::string &csv_path);

    // csv 导入为数据库表
    bool overwrite_table_from_csv(const std::string &table_name, const std::string &csv_path);

    // 导出初始化成功的数据库表
    static std::vector<std::string> list_data_csv_files(const std::string &dir);

    // 获取某一张表的列，便于交集导入csv，而不是覆盖制
    std::vector<std::string> get_table_columns(const std::string &table_name);

};


#endif //BIBO_NO_AOZORA_DATABASECHECKER_H
