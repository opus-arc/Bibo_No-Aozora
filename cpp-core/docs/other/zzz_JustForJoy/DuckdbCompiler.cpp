//
// Created by opus arc on 2026/1/29.
//

#include "DuckdbCompiler.h"

#include "../../../utils/internal/SpectralLoom/SpectralLoom.h"

#include <filesystem>

// duckdb 的工作域相对位置
#define DUCKDB_WORKSPACE "/Bibo_no-Aozora/cmake-build-debug"

// course.csv 文件的相对位置
#define COURSE_PATH "../backend/database/course.csv"

// history.csv 文件的相对位置
#define USER_PATH "../backend/database/history.csv"

/**
 * CSV → DuckDB 数据编译与校验层。
 *
 * DuckDB 数据增强，更接近服务型数据库的基础功能
 */
DuckdbCompiler::DuckdbCompiler() {
    /**
     * 一、
     * csv duckdb 存在性检查与数据同步
     * 该操作在构造阶段自动执行
     * 当 course.csv 与 course.duckdb 数据不一致时，以 course.csv 为准。
     * 当 use.csv 与 user.duckdb 数据不一致时，以 user.duckdb 为准。
     *
     * 说明：
     * - course.csv 为纯人工维护的主数据表，故以 csv 为事实源
     * - history.csv 为程序生成的纯派生数据表（只读），故以 duckdb 为事实源
     * - 表不存在时创建，存在时重建
     */

    // course.csv 的硬性存在性检查
    if (!std::filesystem::exists(COURSE_PATH))
        throw std::runtime_error("course.csv file does not exist");

    // user.duckdb 的软性存在性检查 （若不存在则按照下述模板进行生成）
    databaseDB_con.Query(R"SQL(
        CREATE TABLE IF NOT EXISTS "user" (
            id    VARCHAR,
            name  VARCHAR,

            mode      VARCHAR,
            interval  DOUBLE,

            l_reviewed_at TIMESTAMP,
            c_reviewed_at TIMESTAMP,

            l_performance DOUBLE,
            c_performance DOUBLE,

            l_difficulty DOUBLE,
            c_difficulty DOUBLE,

            l_half_life DOUBLE,
            c_half_life DOUBLE,

            l_retrieval_probability DOUBLE,
            c_retrieval_probability DOUBLE,

            ivl    DOUBLE,
            recall DOUBLE,
            cost   DOUBLE
        );
    )SQL");

    // 以 course.csv 为唯一事实源强制覆盖 course.duckdb 表单
    databaseDB_con.Query(
        "CREATE OR REPLACE TABLE course AS "
        "SELECT * "
        "FROM read_csv_auto('" COURSE_PATH "');"
    );

    // 以 user.duckdb 为唯一事实源导出 CSV 快照 强制覆盖 history.csv 路径
    databaseDB_con.Query(
        "COPY (SELECT * FROM \"user\") "
        "TO '" USER_PATH "' (FORMAT CSV, HEADER, DELIMITER ',');"
    );

    /**
     * 二、
     * duckdb 合法性检查
     *
     * 说明：
     *  - 表头与数据类型
     *  - 数学合理性
     */

    // course user 表头与数据类型检查
    try {
        format_check_course();
        format_check_user();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }


}

// course 表的表头与数据类型检查（适配：id/pid 可能带前导 0 -> VARCHAR；R 等为 DOUBLE）
void DuckdbCompiler::format_check_course() {
    struct Col {
        const char *name;
        std::vector<const char *> types; // acceptable information_schema.columns.data_type
    };

    const std::vector<Col> expected = {
        {"id",      {"VARCHAR"}},               // 01 / 0101 ... 带前导 0，应该是字符串
        {"version", {"INTEGER", "BIGINT", "VARCHAR"}},     // 允许为空
        {"name",    {"VARCHAR"}},
        {"note",    {"VARCHAR"}},
        {"pid",     {"VARCHAR"}},               // 01 这类父 id，仍建议 VARCHAR
        {"pname",   {"VARCHAR"}},

        {"R",       {"DOUBLE", "REAL", "DECIMAL"}},
        {"r_1",       {"DOUBLE", "REAL", "DECIMAL", "VARCHAR"}},
        {"a",       {"DOUBLE", "REAL", "DECIMAL", "VARCHAR"}},
        {"h",       {"DOUBLE", "REAL", "DECIMAL", "VARCHAR"}},
        {"t",       {"DOUBLE", "REAL", "DECIMAL", "VARCHAR"}},
        {"A_1",       {"DOUBLE", "REAL", "DECIMAL", "VARCHAR"}}
    };

    auto res = databaseDB_con.Query(R"SQL(
        SELECT column_name, data_type
        FROM information_schema.columns
        WHERE table_schema = 'main' AND table_name = 'course'
        ORDER BY ordinal_position
    )SQL");

    if (!res || res->HasError()) {
        throw std::runtime_error("course 表结构检查失败：无法读取表结构");
    }

    if (res->RowCount() != expected.size()) {
        throw std::runtime_error("course 表结构检查失败：列数不匹配");
    }

    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    };

    auto type_ok = [&](const std::string &actual_type, const std::vector<const char*> &acceptable) {
        const auto a = lower(actual_type);
        for (auto *t : acceptable) {
            if (a == lower(std::string(t))) return true;
        }
        return false;
    };

    for (idx_t i = 0; i < expected.size(); ++i) {
        const std::string actual_name = res->GetValue(0, i).ToString();
        const std::string actual_type = res->GetValue(1, i).ToString();

        if (lower(actual_name) != lower(expected[i].name)) {
            throw std::runtime_error(
                "course 表结构错误：第 " + std::to_string(i + 1) +
                " 列名不匹配（期望 " + std::string(expected[i].name) +
                "，实际 " + actual_name + "）"
            );
        }

        if (!type_ok(actual_type, expected[i].types)) {
            std::string expect_types;
            for (size_t k = 0; k < expected[i].types.size(); ++k) {
                if (k) expect_types += "/";
                expect_types += expected[i].types[k];
            }
            throw std::runtime_error(
                "course 表结构错误：列 " + actual_name +
                " 类型不匹配（期望 " + expect_types +
                "，实际 " + actual_type + "）"
            );
        }
    }
}

// user 表的表头与数据类型检查（适配新表头：mode/interval/l_*/c_* 等）
void DuckdbCompiler::format_check_user() {
    struct Col {
        const char *name;
        std::vector<const char *> types; // acceptable information_schema.columns.data_type
    };

    const std::vector<Col> expected = {
        {"id",   {"VARCHAR"}},                      // 0207 这类通常希望保留前导 0
        {"name", {"VARCHAR"}},

        {"mode", {"VARCHAR"}},                      // R / L / ... 用字符串更稳
        {"interval", {"INTEGER", "BIGINT", "DOUBLE"}}, // CSV 推断可能是整型或浮点

        {"l_reviewed_at", {"DATE", "TIMESTAMP", "TIMESTAMP WITH TIME ZONE", "VARCHAR"}},
        {"c_reviewed_at", {"DATE", "TIMESTAMP", "TIMESTAMP WITH TIME ZONE", "VARCHAR"}},

        // 你示范数据里 l_performance/c_performance 有“空/布尔/日期”等混合的可能；
        // DuckDB 很容易把这种列推断成 VARCHAR，所以这里放宽到 VARCHAR/DOUBLE/BOOLEAN。
        {"l_performance", {"VARCHAR", "DOUBLE", "REAL", "DECIMAL", "BOOLEAN"}},
        {"c_performance", {"VARCHAR", "DOUBLE", "REAL", "DECIMAL", "BOOLEAN"}},

        {"l_difficulty", {"DOUBLE", "REAL", "DECIMAL"}},
        {"c_difficulty", {"DOUBLE", "REAL", "DECIMAL"}},

        {"l_half_life", {"DOUBLE", "REAL", "DECIMAL"}},
        {"c_half_life", {"DOUBLE", "REAL", "DECIMAL"}},

        {"l_retrieval_probability", {"DOUBLE", "REAL", "DECIMAL"}},
        {"c_retrieval_probability", {"DOUBLE", "REAL", "DECIMAL"}},

        {"ivl",    {"DOUBLE", "REAL", "DECIMAL"}},
        {"recall", {"DOUBLE", "REAL", "DECIMAL"}},
        {"cost",   {"DOUBLE", "REAL", "DECIMAL"}}
    };

    auto res = databaseDB_con.Query(R"SQL(
        SELECT column_name, data_type
        FROM information_schema.columns
        WHERE table_schema = 'main' AND table_name = 'user'
        ORDER BY ordinal_position
    )SQL");

    if (!res || res->HasError()) {
        throw std::runtime_error("user 表结构检查失败：无法读取表结构（user 表可能不存在）");
    }

    if (res->RowCount() != expected.size()) {
        throw std::runtime_error(
            "user 表结构检查失败：列数不匹配（期望 " + std::to_string(expected.size()) +
            "，实际 " + std::to_string(res->RowCount()) + "）"
        );
    }

    auto lower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    };

    auto type_ok = [&](const std::string &actual_type, const std::vector<const char*> &acceptable) {
        const auto a = lower(actual_type);
        for (auto *t : acceptable) {
            if (a == lower(std::string(t))) return true;
        }
        return false;
    };

    for (idx_t i = 0; i < expected.size(); ++i) {
        const std::string actual_name = res->GetValue(0, i).ToString();
        const std::string actual_type = res->GetValue(1, i).ToString();

        if (lower(actual_name) != lower(expected[i].name)) {
            throw std::runtime_error(
                "user 表结构错误：第 " + std::to_string(i + 1) +
                " 列名不匹配（期望 " + std::string(expected[i].name) +
                "，实际 " + actual_name + "）"
            );
        }

        if (!type_ok(actual_type, expected[i].types)) {
            std::string expect_types;
            for (size_t k = 0; k < expected[i].types.size(); ++k) {
                if (k) expect_types += "/";
                expect_types += expected[i].types[k];
            }
            throw std::runtime_error(
                "user 表结构错误：列 " + actual_name +
                " 类型不匹配（期望 " + expect_types +
                "，实际 " + actual_type + "）"
            );
        }
    }
}




// f(basicDifficulty) -> d0[unsigned int 1 ~ 18] / 经验难度 -> 难度初始值 / 音频难度编译算法
// f(d0) -> h0[double 0 ~ 360] / 难度初始值 -> 半衰期初始值 / h_0 = -\frac{1}{\log_2\!\left(0.925 - 0.05 \cdot d_0\right)}
// f(d0, h0) -> ivl[int 1 ~ 360], cost[double 0 ~ ?], recall[double 0 ~ 1]  / 由 d0, h0 连立查表近似得出
// f(ivl, recall) -> card / ivl == 1 && recall 升序
// f(card.name) -> audio
// f(audio，card.name) -> o0[string “认识” "模糊" “不认识”]，trust[TO DO] / (output) 用户作答 得到包含模糊的结果和信任度
// f(o0, trust) -> o[bool] / 将包括模糊的结果编译成布尔值结果
// f(o, recall, d0, h0) -> d1, h1
// f(d1, h1) -> ivl, cost, recall / 由 d1, h1 连立查表近似得出

// ssp-mmc 负责选牌， anki 负责把牌打完
















// ---------------------------------------------
// demo 加急方案
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

// 先列出到期卡
void DuckdbCompiler::demoCardsQuery() {
    auto course_res = databaseDB_con.Query(R"SQL(
        SELECT id, name, R
        FROM course
        WHERE pid = '0102'
    )SQL");

    if (!course_res || course_res->HasError()) {
        throw std::runtime_error("demoCardsQuery 查询失败");
    }

    auto check_user = databaseDB_con.Prepare(R"SQL(
        SELECT 1 FROM "user" WHERE id = ?
    )SQL");
    if (!check_user || check_user->HasError()) {
        throw std::runtime_error("Prepare check_user 失败");
    }

    auto insert_user = databaseDB_con.Prepare(R"SQL(
        INSERT INTO "user" (
            id,
            name,
            l_difficulty,
            mode,
            "interval",
            c_reviewed_at,
            l_half_life,
            l_retrieval_probability
        )
        VALUES (?, ?, ?, 'R', 365, ?, ?, ?)
    )SQL");
    if (!insert_user || insert_user->HasError()) {
        throw std::runtime_error("Prepare insert_user 失败");
    }

    for (idx_t i = 0; i < course_res->RowCount(); ++i) {
        auto id   = course_res->GetValue(0, i);
        auto name = course_res->GetValue(1, i);
        auto r    = course_res->GetValue(2, i); // d

        // 查 user 是否已有该 id
        duckdb::vector<duckdb::Value> check_params;
        check_params.push_back(id);

        auto exists = check_user->Execute(check_params);
        if (!exists || exists->HasError()) {
            throw std::runtime_error("user 表查询失败");
        }

        auto chunk = exists->Fetch();
        if (!chunk || chunk->size() == 0) {
            /* ---------- c_reviewed_at（中国时间 YYYY-MM-DD） ---------- */
            auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
            std::time_t tt = std::chrono::system_clock::to_time_t(now);
            std::tm tm_utc = *std::gmtime(&tt);

            std::ostringstream oss;
            oss << std::put_time(&tm_utc, "%Y-%m-%d");
            std::string c_reviewed_at = oss.str();

            /* ---------- l_half_life ---------- */
            double d = r.GetValue<double>();
            double half_life = 0.20 * std::pow(0.05, (d - 1.0) / 17.0);

            /* ---------- l_retrieval_probability 回忆概率 0 ～ 1 8位 double ---------- */
            constexpr double interval = 360.0;
            double retrieval_p = std::pow(2.0, -interval / half_life);

            /* ---------- insert ---------- */
            duckdb::vector<duckdb::Value> ins_params;
            ins_params.push_back(id);
            ins_params.push_back(name);
            ins_params.push_back(r); // l_difficulty
            ins_params.push_back(duckdb::Value(c_reviewed_at));
            ins_params.push_back(duckdb::Value::DOUBLE(half_life));
            ins_params.push_back(duckdb::Value::DOUBLE(retrieval_p));

            auto ins = insert_user->Execute(ins_params);
            if (!ins || ins->HasError()) {
                throw std::runtime_error("user 表插入失败");
            }
        }
    }
}

void DuckdbCompiler::demoPlayLowestDifficultyUser() {
    // 取 l_difficulty 最小的那条记录的 name
    auto res = databaseDB_con.Query(R"SQL(
        SELECT name
        FROM "user"
        ORDER BY l_difficulty ASC
        LIMIT 1
    )SQL");

    if (!res || res->HasError()) {
        throw std::runtime_error("playLowestDifficultyUser 查询失败");
    }

    // 空表就直接结束
    auto chunk = res->Fetch();
    if (!chunk || chunk->size() == 0) {
        return;
    }

    // chunk 的第 0 列第 0 行就是 name
    std::string name = chunk->GetValue(0, 0).ToString();

    // 用 name 调用播放
    SpectralLoom::playInterval(name);
}

void DuckdbCompiler::demoUpdate(const std::string &input) {
    // 1) 取出 l_difficulty 最低的用户：c_name + 当前 d/h/P
    auto sel_res = databaseDB_con.Query(R"SQL(
        SELECT name, l_difficulty, l_half_life, l_retrieval_probability
        FROM "user"
        ORDER BY l_difficulty ASC
        LIMIT 1
    )SQL");

    if (!sel_res || sel_res->HasError()) {
        throw std::runtime_error("demoUpdate 查询 user 失败");
    }

    auto chunk = sel_res->Fetch();
    if (!chunk || chunk->size() == 0) {
        return; // user 空表
    }

    std::string c_name = chunk->GetValue(0, 0).ToString();
    double d = chunk->GetValue(1, 0).GetValue<double>(); // l_difficulty
    double h = chunk->GetValue(2, 0).GetValue<double>(); // l_half_life
    double P = chunk->GetValue(3, 0).GetValue<double>(); // l_retrieval_probability

    // 2) 比较 input 与 c_name，得到 r(0/1) 与 c_performance
    bool c_performance = (input == c_name);
    int r = c_performance ? 1 : 0;

    // 3) 更新难度 d_next
    double d_next = d;
    if (r == 0) d_next = std::min(10.0, d + 0.1);
    else        d_next = std::max(1.0,  d - 0.05);

    // 4) 更新半衰期 h_next（按图上公式）
    double h_next = h;
    constexpr double K_scale = 1.0; // 若你有明确取值，改这里

    if (r == 1) {
        double Gain = K_scale
                      * std::exp(3.81)
                      * std::pow(d_next, -0.534)
                      * std::pow(h,      -0.127)
                      * std::pow(1.0 - P, 0.970);
        h_next = h * (1.0 + Gain);
    } else {
        double Retention = std::exp(-0.041)
                           * std::pow(d_next, -0.041)
                           * std::pow(h,       0.377)
                           * std::pow(1.0 - P, -0.227);
        h_next = h * Retention;
    }

    // 5) 用新半衰期算新的 l_retrieval_probability：P = 2^(-t/h)
    constexpr double interval = 365.0;
    double P_next = std::pow(2.0, -interval / h_next);

    // 6) 中国时间当天日期 YYYY-MM-DD（如你需要同步更新 reviewed_at）
    auto now = std::chrono::system_clock::now() + std::chrono::hours(8);
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm_utc = *std::gmtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&tm_utc, "%Y-%m-%d");
    std::string c_reviewed_at = oss.str();

    // 7) 更新 user 表（h -> c_half_life, h_next -> l_half_life）
    auto upd_stmt = databaseDB_con.Prepare(R"SQL(
        UPDATE "user"
        SET
            c_performance = ?,
            c_half_life = ?,
            l_difficulty = ?,
            l_half_life = ?,
            l_retrieval_probability = ?,
            c_reviewed_at = ?
        WHERE name = ?
    )SQL");
    if (!upd_stmt || upd_stmt->HasError()) {
        throw std::runtime_error("demoUpdate Prepare update 失败");
    }

    duckdb::vector<duckdb::Value> params;
    params.push_back(duckdb::Value::BOOLEAN(c_performance));
    params.push_back(duckdb::Value::DOUBLE(h));        // c_half_life = old h
    params.push_back(duckdb::Value::DOUBLE(d_next));   // l_difficulty
    params.push_back(duckdb::Value::DOUBLE(h_next));   // l_half_life
    params.push_back(duckdb::Value::DOUBLE(P_next));   // l_retrieval_probability
    params.push_back(duckdb::Value(c_reviewed_at));    // c_reviewed_at
    params.push_back(duckdb::Value(c_name));           // WHERE name = c_name

    auto upd_res = upd_stmt->Execute(params);
    if (!upd_res || upd_res->HasError()) {
        throw std::runtime_error("demoUpdate 更新 user 失败");
    }
}
