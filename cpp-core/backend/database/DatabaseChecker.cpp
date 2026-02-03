//
// Created by opus arc on 2026/1/31.
//

#include "DatabaseChecker.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

// ----------------------------------- 工具函数签名 -------------------------------------------
/**
 * @param data_dir     数据 csv 文件夹
 * @param template_dir 模板文件夹
 * @throws std::runtime_error 当模板文件不存在时抛异常
 */
void check_csv_templates(const std::string &data_dir, const std::string &template_dir);


/**
 * @brief 去除字符串首尾的空白字符
 *
 * 空白字符的定义遵循 C 标准库的 `std::isspace`，
 * 包括空格、制表符、换行符等。
 *
 * @param s 输入字符串（按值传入，内部会修改）
 * @return 去除首尾空白后的字符串
 */
static inline std::string trim(std::string s);

/**
 * @brief 判断字符串是否是“简单 SQL 标识符”
 *
 * 简单标识符定义：
 * - 以字母或下划线开头
 * - 仅包含字母、数字、下划线
 *
 * 满足该条件的标识符可在 SQL 中直接使用，
 * 否则需要使用双引号进行引用。
 *
 * @param s 待判断的字符串
 * @return true  如果是简单标识符
 * @return false 如果包含特殊字符或为空
 */
static bool is_simple_identifier(const std::string &s);

/**
 * @brief 若需要，为 SQL 标识符添加双引号引用
 *
 * 当标识符是简单标识符（见 is_simple_identifier）时，
 * 原样返回；否则使用双引号进行包裹，并对内部双引号做转义。
 *
 * 该函数用于安全生成 SQL 语句中的表名、列名，
 * 避免因特殊字符或关键字导致语法错误。
 *
 * @param ident 原始标识符字符串
 * @return 可直接用于 SQL 的标识符表示
 */
static std::string quote_ident_if_needed(const std::string &ident);

/**
 * @brief 将区间描述字符串转换为 SQL CHECK 约束表达式
 *
 * 支持的区间格式示例：
 * - "[1,18]"  -> col >= 1 AND col <= 18
 * - "(0,1]"   -> col > 0 AND col <= 1
 * - "[0,)"    -> col >= 0
 * - "(,360]"  -> col <= 360
 *
 * 说明：
 * - 方括号 [] 表示闭区间
 * - 圆括号 () 表示开区间
 * - 空边界表示无穷
 * - 若 range_raw 为空或无法解析，则返回 std::nullopt
 *
 * @param col_sql_ident 已处理好的列 SQL 标识符（可直接用于 SQL）
 * @param range_raw 模板中的 range 原始字符串
 * @return SQL CHECK 条件字符串（不含 CHECK 关键字），或 std::nullopt
 */
static std::optional<std::string> range_to_check(const std::string &col_sql_ident, const std::string &range_raw);

/**
 * @brief 若字符串以指定后缀结尾，则去除该后缀
 *
 * 常用于从模板表名或模板文件名中推导实例名称：
 * 例如：
 * - "course_Template" 去掉 "_Template" -> "course"
 *
 * 若字符串不以 suffix 结尾，则原样返回。
 *
 * @param s 原始字符串
 * @param suffix 需要去除的后缀
 * @return 去除后缀后的字符串，或原字符串
 */
static std::string strip_suffix(const std::string &s, const std::string &suffix);

/**
 * @param dir 模板文件夹路径
 * @return 所有 .template.csv 文件的路径（不递归）
 */
std::vector<std::string> list_template_csv_files(const std::string &dir);


// ----------------------------------- 具体功能实现 -------------------------------------------

/**
 * 扫描 generated 文件夹下的 csv 文件与数据库中的 duckdb 文件
 * 按照 templates 文件夹下的模版进行表头检查与类型检查或在不存在时按照模板覆盖路径
 */
DatabaseChecker::DatabaseChecker() {
    std::cout << "\n[DatabaseChecker]: " << "数据库检查开始！" << std::endl;
    try {
        // 确保数据库作为空的 csv 操作容器
        std::cout << "[DatabaseChecker]: " << "确保数据库作为空的 csv 操作容器" << std::endl;
        clearDatabase();

        // templates generated manaul 文件夹的存在性检查
        std::cout << "[DatabaseChecker]: " << "templates generated manaul 文件夹的存在性检查" << std::endl;
        existChecker();

        // 确保模版文件完整
        std::cout << "[DatabaseChecker]: " << "确保模版文件完整" << std::endl;
        check_csv_templates(GENERATED_FOLDER_PATH, TEMPLATES_FOLDER_PATH);
        check_csv_templates(MANUAL_FOLDER_PATH, TEMPLATES_FOLDER_PATH);

        // templates 优先输导入数据库 保存模版名字 实例化模版
        std::cout << "[DatabaseChecker]: " << "templates 优先输导入数据库 保存模版名字 实例化模版" << std::endl;
        for (const auto &templateTableName: templatesPorter())
            templatesComplier(templateTableName);

        // 交集输入 manual 与 generated 下的所有 csv 数据
        std::cout << "[DatabaseChecker]: " << "交集输入 manual 与 generated 下的所有 csv 数据" << std::endl;
        for (const auto &csv_file_path: list_data_csv_files(MANUAL_FOLDER_PATH)) {
            std::string tableName = std::filesystem::path(csv_file_path).stem().stem().string();
            if (!overwrite_table_from_csv(tableName, csv_file_path))
                throw std::runtime_error("manual 文件夹下存在非法csv文件");
        }
        for (const auto &csv_file_path: list_data_csv_files(GENERATED_FOLDER_PATH)) {
            std::string tableName = std::filesystem::path(csv_file_path).stem().stem().string();
            if (!overwrite_table_from_csv(tableName, csv_file_path)) {
                export_table_to_csv(tableName, csv_file_path);
                throw std::runtime_error("manual 文件夹下存在非法csv文件");
            }
        }
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::cout << "[DatabaseChecker]: " << "数据库检查完毕！" << std::endl;
};

DatabaseChecker::~DatabaseChecker() = default;

// ----------------------------------- 功能函数实现 -------------------------------------------
/**
 * 清空数据库
 * @throws std::runtime_error 当没卸载成功时抛异常
 */
void DatabaseChecker::clearDatabase() {
    const auto tables = databaseDB_con.Query(R"SQL(
        SELECT table_schema, table_name
        FROM information_schema.tables
        WHERE table_schema = 'main'
          AND table_type = 'BASE TABLE'
        ORDER BY table_schema, table_name;
    )SQL");

    if (!tables || tables->HasError()) {
        throw std::runtime_error(
            "list tables failed: " +
            (tables ? tables->GetError() : "null")
        );
    }

    for (duckdb::idx_t i = 0; i < tables->RowCount(); ++i) {
        const std::string schema = tables->GetValue(0, i).ToString();
        const std::string table = tables->GetValue(1, i).ToString();

        const std::string sql =
                "DROP TABLE IF EXISTS \"" + schema + "\".\"" += table + "\";";

        const auto drop = databaseDB_con.Query(sql);

        if (!drop || drop->HasError()) {
            throw std::runtime_error(
                "drop table failed for " + schema + "." += table + ": " +
                (drop ? drop->GetError() : "null")
            );
        }
    }
}

/**
 * 存在性检查
 * @throws std::runtime_error 文件夹不存在与模版文件不全面时抛异常
 */
void DatabaseChecker::existChecker() {
    if (!std::filesystem::exists(TEMPLATES_FOLDER_PATH))
        throw std::runtime_error("不存在 templates folder");
    if (!std::filesystem::exists(GENERATED_FOLDER_PATH))
        throw std::runtime_error("不存在 generated folder");
    if (!std::filesystem::exists(MANUAL_FOLDER_PATH))
        throw std::runtime_error("不存在 manual folder");

    // 确保所有已存在的表都有模版
    try {
        check_csv_templates(GENERATED_FOLDER_PATH, TEMPLATES_FOLDER_PATH);
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief 从一个模板表（name,type,range,note）生成实例表
 * @param template_table_name 例如 "course_Template"
 * @param suffix 模板后缀，默认 "_Template"
 * @throws std::runtime_error 没有实例化成功时抛异常
 */
void DatabaseChecker::templatesComplier(const std::string &template_table_name, const std::string &suffix) {
    const std::string base = strip_suffix(template_table_name, suffix);

    const std::string tmpl_ident = quote_ident_if_needed(template_table_name);
    const std::string base_ident = quote_ident_if_needed(base);

    // 读取模板表（你模板表已经是结构化数据了，直接 SELECT）
    auto res = databaseDB_con.Query(
        "SELECT name, type, range "
        "FROM " + tmpl_ident + " "
        "ORDER BY rowid;"
    );

    if (!res || res->HasError()) {
        throw std::runtime_error("读取模板表失败(" + template_table_name + "): " +
                                 (res ? res->GetError() : "null"));
    }
    if (res->RowCount() == 0) {
        throw std::runtime_error("模板表为空，无法建实例表: " + template_table_name);
    }

    std::string ddl = "CREATE OR REPLACE TABLE " + base_ident + " (\n";

    bool has_primary_key = false;

    for (duckdb::idx_t i = 0; i < res->RowCount(); ++i) {
        std::string col_name = trim(res->GetValue(0, i).ToString());
        std::string col_type = trim(res->GetValue(1, i).ToString());

        std::string col_range;
        if (auto v_range = res->GetValue(2, i); !v_range.IsNull()) {
            col_range = trim(v_range.ToString());
        }

        if (col_name.empty() || col_type.empty()) {
            throw std::runtime_error("模板表(" + template_table_name + ") 第 " +
                                     std::to_string((size_t) i) + " 行 name/type 为空");
        }

        std::string col_ident = quote_ident_if_needed(col_name);

        ddl += "  " + col_ident + " " + col_type;

        // ⭐ 自动主键
        if (!has_primary_key && col_name == "id") {
            ddl += " PRIMARY KEY";
            has_primary_key = true;
        }

        // CHECK 约束
        if (auto chk = range_to_check(col_ident, col_range); chk.has_value()) {
            ddl += " CHECK (" + chk.value() + ")";
        }

        ddl += (i + 1 < res->RowCount()) ? ",\n" : "\n";
    }

    ddl += ");";

    if (auto create_res = databaseDB_con.Query(ddl); !create_res || create_res->HasError()) {
        throw std::runtime_error("创建实例表失败(" + base + "): " +
                                 (create_res ? create_res->GetError() : "null"));
    }
}

/**
 * 模版csv文件入库
 * @throws std::runtime_error 没有入库成功时抛异常
 */
std::vector<std::string> DatabaseChecker::templatesPorter() {
    std::vector<std::string> template_table_names;
    for (const std::string &csvTemplatePathStr: list_template_csv_files(TEMPLATES_FOLDER_PATH)) {
        std::filesystem::path csvPath(csvTemplatePathStr);

        // name.template.csv -> name
        std::string tableName = csvPath.stem().stem().string() + "_Template";

        // 保存模版名字
        template_table_names.push_back(tableName);

        std::string sql =
                "CREATE OR REPLACE TABLE " + tableName + " AS "
                "SELECT * FROM read_csv_auto('" += csvTemplatePathStr + "');";

        databaseDB_con.Query(sql);
    }
    return template_table_names;
}

/**
 * @brief 将指定表的数据导出为 CSV 文件
 *
 * 使用 DuckDB 的 COPY TO 语句，将整张表（含表头）
 * 导出为 CSV 文件，若目标文件已存在则覆盖。
 *
 * @param table_name 数据表名
 * @param csv_path 输出 CSV 文件路径
 */
bool DatabaseChecker::export_table_to_csv(const std::string &table_name,
                                          const std::string &csv_path) {
    std::string sql =
            "COPY " + table_name +
            " TO '" + csv_path +
            "' (HEADER, DELIMITER ',');";

    auto res = databaseDB_con.Query(sql);

    if (!res) {
        std::cerr << "[DatabaseChecker] 导出 CSV 失败(" << table_name
                << "): null result\n";
        return false;
    }

    if (res->HasError()) {
        std::cerr << "[DatabaseChecker] 导出 CSV 失败(" << table_name
                << "): " << res->GetError() << "\n";
        return false;
    }

    return true;
}


/**
 * @brief 使用 CSV 文件的数据覆写指定表 后改为交集导入
 *
 * 行为：
 * - 若表已存在，则删除
 * - 使用 read_csv_auto 读取 CSV
 * - 自动推断 schema 并创建新表
 *
 * @param table_name 目标表名
 * @param csv_path CSV 文件路径
 */
bool DatabaseChecker::overwrite_table_from_csv(const std::string &table_name,
                                               const std::string &csv_path) {
    const std::vector<std::string> columns = get_table_columns(table_name);

    std::string col_list;
    for (size_t i = 0; i < columns.size(); ++i) {
        col_list += columns[i];
        if (i + 1 < columns.size()) col_list += ", ";
    }

    const std::string sql =
            "INSERT INTO " + table_name + " (" + col_list + ") "
            "SELECT " + col_list + " FROM read_csv_auto('" + csv_path + "');";

    const auto res = databaseDB_con.Query(sql);

    if (!res || res->HasError()) {
        std::cerr << "从 csv 交集写入数据库失败: "
                << (res ? res->GetError() : "Query 返回 nullptr")
                << std::endl;
    }
    return res && !res->HasError();
}

std::vector<std::string> DatabaseChecker::get_table_columns(const std::string &table_name) {
    const auto res = databaseDB_con.Query("PRAGMA table_info('" + table_name + "');");
    if (!res || res->HasError()) {
        throw std::runtime_error("读取表结构失败: " +
                                 (res ? res->GetError() : "null"));
    }

    std::vector<std::string> cols;
    cols.reserve(res->RowCount());
    for (duckdb::idx_t i = 0; i < res->RowCount(); ++i) {
        cols.push_back(res->GetValue(1, i).ToString()); // name
    }
    return cols;
}

std::vector<std::string> DatabaseChecker::list_data_csv_files(const std::string &dir) {
    namespace fs = std::filesystem;
    std::vector<std::string> result;

    fs::path p(dir);
    if (!fs::exists(p) || !fs::is_directory(p)) return result;

    for (const auto &e: fs::directory_iterator(p)) {
        if (!e.is_regular_file()) continue;
        const auto &f = e.path();

        // 只要 .csv，但排除 .template.csv
        if (f.extension() == ".csv" && f.stem().extension() != ".template") {
            result.push_back(f.string());
        }
    }
    return result;
}


// ----------------------------------- 工具函数实现 -------------------------------------------


/**
 * @param data_dir     数据 csv 文件夹
 * @param template_dir 模板文件夹
 * @throws std::runtime_error 当模板文件不存在时抛异常
 */
void check_csv_templates(const std::string &data_dir, const std::string &template_dir) {
    const std::filesystem::path data_path(data_dir);
    const std::filesystem::path template_path(template_dir);

    if (!std::filesystem::exists(data_path) || !std::filesystem::is_directory(data_path)) {
        throw std::runtime_error("数据文件夹不存在或不是目录");
    }

    if (!std::filesystem::exists(template_path) || !std::filesystem::is_directory(template_path)) {
        throw std::runtime_error("模板文件夹不存在或不是目录");
    }

    for (const auto &entry: std::filesystem::directory_iterator(data_path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::filesystem::path &csv_path = entry.path();
        if (csv_path.extension() != ".csv") {
            continue;
        }

        // 构造模板文件名：xxx.template.csv
        std::filesystem::path template_file =
                template_path /
                (csv_path.stem().string() + ".template.csv");

        if (!std::filesystem::exists(template_file)) {
            throw std::runtime_error(
                "该 csv 的模板文件不存在: " +
                template_file.string());
        }
    }
}

/**
 * @param dir 模板文件夹路径
 * @return 所有 .template.csv 文件的路径（不递归）
 */
std::vector<std::string> list_template_csv_files(const std::string &dir) {
    namespace fs = std::filesystem;

    std::vector<std::string> result;
    fs::path path(dir);

    if (!fs::exists(path) || !fs::is_directory(path)) {
        return result;
    }

    for (const auto &entry: fs::directory_iterator(path)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const fs::path &p = entry.path();
        if (p.extension() == ".csv" &&
            p.stem().extension() == ".template") {
            result.push_back(p.string());
        }
    }

    return result;
}


static std::string trim(std::string s) {
    auto is_ws = [](unsigned char c) { return std::isspace(c); };
    while (!s.empty() && is_ws((unsigned char) s.front())) s.erase(s.begin());
    while (!s.empty() && is_ws((unsigned char) s.back())) s.pop_back();
    return s;
}

static bool is_simple_identifier(const std::string &s) {
    if (s.empty()) return false;
    auto is_alpha_ = [](unsigned char c) { return std::isalpha(c) || c == '_'; };
    auto is_alnum_ = [](unsigned char c) { return std::isalnum(c) || c == '_'; };
    if (!is_alpha_((unsigned char) s[0])) return false;
    for (size_t i = 1; i < s.size(); ++i) if (!is_alnum_((unsigned char) s[i])) return false;
    return true;
}

static std::string quote_ident_if_needed(const std::string &ident) {
    if (is_simple_identifier(ident)) return ident;
    std::string out = "\"";
    for (char c: ident) out += (c == '"') ? "\"\"" : std::string(1, c);
    out += "\"";
    return out;
}

static std::optional<std::string> range_to_check(const std::string &col_sql_ident,
                                                 const std::string &range_raw) {
    std::string r = trim(range_raw);
    if (r.empty()) return std::nullopt;

    // 兼容仍带引号
    if (r.size() >= 2 && ((r.front() == '"' && r.back() == '"') || (r.front() == '\'' && r.back() == '\''))) {
        r = trim(r.substr(1, r.size() - 2));
    }
    if (r.size() < 3) return std::nullopt;

    char L = r.front(), R = r.back();
    if (!((L == '[') || (L == '('))) return std::nullopt;
    if (!((R == ']') || (R == ')'))) return std::nullopt;

    std::string inside = trim(r.substr(1, r.size() - 2));
    auto comma = inside.find(',');
    if (comma == std::string::npos) return std::nullopt;

    std::string a = trim(inside.substr(0, comma));
    std::string b = trim(inside.substr(comma + 1));

    const bool has_lower = !a.empty();
    const bool has_upper = !b.empty() && b != ")";

    std::vector<std::string> conds;
    if (has_lower) conds.push_back(col_sql_ident + (L == '[' ? " >= " : " > ") + a);
    if (has_upper) conds.push_back(col_sql_ident + (R == ']' ? " <= " : " < ") + b);

    if (conds.empty()) return std::nullopt;

    std::string out;
    for (size_t i = 0; i < conds.size(); ++i) {
        if (i) out += " AND ";
        out += conds[i];
    }
    return out;
}

static std::string strip_suffix(const std::string &s, const std::string &suffix) {
    if (s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
        return s.substr(0, s.size() - suffix.size());
    }
    return s; // 不匹配就原样返回（也可以改成抛错）
}
