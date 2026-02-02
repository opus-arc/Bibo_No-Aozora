//
// Created by opus arc on 2026/1/29.
//

#ifndef BIBO_NO_AOZORA_DUCKDBCOMPILER_H
#define BIBO_NO_AOZORA_DUCKDBCOMPILER_H

#include "../../../utils/external/duckdb/duckdb.hpp"
#define DATABASE_PATH "/databaseDB.duckdb"

/**
 * DuckDB 的轻量封装。
 *
 * - 输入：原生 SQL 字符串
 * - 输出：字符串化的查询结果
 *
 * 配合 DuckDB 实现用原生 SQL 语句操作 csv 文件的同时
 * 不暴露 DuckDB 原生对象，用于隔离底层实现。
 *
 */
class DuckdbCompiler {

    public:

    // 连接 duckdb 数据库
    duckdb::DuckDB databaseDB{DATABASE_PATH};
    duckdb::Connection databaseDB_con{databaseDB};

    DuckdbCompiler();
    // ~DuckdbCompiler();

    // course 表的表头与数据类型检查
    void format_check_course();

    // user 表的表头与数据类型检查
    void format_check_user();

    // 数据库 sql 查询简易封装
    // std::string repository_query(std::string query);

    // ----------------------------------------
    // demo 急用
    // 初始化 user 表
    void demoCardsQuery();
    // 播放最低难度的音频
    void demoPlayLowestDifficultyUser();
    // 根据答案更新用户数据
    void demoUpdate(const std::string &input);

};


#endif //BIBO_NO_AOZORA_DUCKDBCOMPILER_H