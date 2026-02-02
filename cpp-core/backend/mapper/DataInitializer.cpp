//
// Created by opus arc on 2026/1/31.
//

#include "DataInitializer.h"

#include "../database/DatabaseChecker.h"

#include "../../utils/internal/SSP_MMC/SSP_MMC.h"


DataInitializer::DataInitializer() {
    std::cout << "\n[DataInitializer]: " << "数据初始化开始！" << std::endl;

    // 初次编译 course 表，算出重要的启动数据
    std::cout << "[DataInitializer]: " << "初次编译 course 表，算出重要的启动数据" << std::endl;
    courseComplier();

    // 初次检查 learningData 表，合并 courseComplier 可能存在的新数据
    std::cout << "[DataInitializer]: " << "初次检查 learningData 表，合并 courseComplier 可能存在的新数据" << std::endl;
    learningDataInitializer();

    // 导出所有初始化成功的数据库表
    std::cout << "[DataInitializer]: " << "导出所有初始化成功的数据库表" << std::endl;
    export_all_tables_to_csvs();

    std::cout << "[DataInitializer]: " << "数据初始化结束！" << std::endl;
}

DataInitializer::~DataInitializer() = default;

// 初次编译 course 表，算出重要的启动数据
void DataInitializer::courseComplier() {

    // 先导入 course 中的所有数据
    const auto result_0 = databaseDB_2_con.Query(R"SQL(
        INSERT INTO courseComplier (id,name,note,pid,pname,basicDifficulty,isLeaf)
        SELECT c.id,c.name,c.note,c.pid,c.pname,c.basicDifficulty,c.isLeaf
        FROM course c
        WHERE NOT EXISTS (
            SELECT 1
            FROM courseComplier cc
            WHERE cc.id = c.id
        );
    )SQL");
    if (!result_0 || result_0->HasError()) {
        std::cerr << "course 表数据迁移失败: "
                << result_0->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    /**
     * 计算 d0,h0,ivl,cost,recall
     */

    // TODO d0
    // f(basicDifficulty) -> d0[unsigned int 1 ~ 18]
    // 经验难度 -> 难度初始值
    // 音频难度编译算法
    const auto result_1 = databaseDB_2_con.Query(R"SQL(
        UPDATE courseComplier
        SET d0 = basicDifficulty + 1
    )SQL");
    if (!result_1 || result_1->HasError()) {
        std::cerr << "d0 更新失败: "
                << result_1->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // h0
    // f(d0) -> h0[double 0 ~ 360]
    // 难度初始值 -> 半衰期初始值
    // h_0 = -\frac{1}{\log_2\!\left(0.925 - 0.05 \cdot d_0\right)}
    const auto result_2 = databaseDB_2_con.Query(R"SQL(
        UPDATE courseComplier
        SET h0 = -1.0 / LOG(2, 0.925 - 0.05 * d0)
    )SQL");
    if (!result_2 || result_2->HasError()) {
        std::cerr << "h0 更新失败: "
                << result_2->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // ivl, cost, recall
    // f(d0, h0) -> ivl[int 1 ~ 360], cost[double 0 ~ ?], recall[double 0 ~ 1]
    // 由 d0, h0 连立查表近似得出
    const auto rows = databaseDB_2_con.Query(R"SQL(
        SELECT id, d0, h0
        FROM courseComplier
    )SQL");
    if (!rows || rows->HasError()) {
        std::cerr << "读取 courseComplier 失败: "
                << (rows ? rows->GetError() : "Query 返回 nullptr")
                << std::endl;
        std::exit(EXIT_FAILURE);
    }
    struct CalcRow {
        int id;
        double ivl;
        double cost;
        double recall;
    };
    std::vector<CalcRow> buffer;
    for (const auto &row: *rows) {
        // 空的就跳过 比如非子节点
        if (row.IsNull(1) || row.IsNull(2))
            continue;

        const auto d0 = row.GetValue<double>(1);
        const auto h0 = row.GetValue<double>(2);
        // std::cout << static_cast<int>(d0) << ", " << h0 << std::endl;
        SSP_MMC::read_SSPMMC_Result("ivl", std::to_string(static_cast<int>(d0)), h0);
        buffer.push_back({
            row.GetValue<int>(0),
            SSP_MMC::read_SSPMMC_Result("ivl", std::to_string(static_cast<int>(d0)), h0),
            SSP_MMC::read_SSPMMC_Result("cost", std::to_string(static_cast<int>(d0)), h0),
            SSP_MMC::read_SSPMMC_Result("recall", std::to_string(static_cast<int>(d0)), h0)
        });
    }
    // 创建临时表
    const auto create_res = databaseDB_2_con.Query(R"SQL(
        CREATE TEMP TABLE tmp_calc (
            id INT PRIMARY KEY,
            ivl DOUBLE,
            cost DOUBLE,
            recall DOUBLE
        );
    )SQL");
    if (!create_res || create_res->HasError()) {
        std::cerr << "临时表创建失败: "
                << create_res->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    // 开事务
    const auto begin_res = databaseDB_2_con.Query("BEGIN TRANSACTION");
    if (!begin_res || begin_res->HasError()) {
        std::cerr << "BEGIN 失败" << std::endl
                << begin_res->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // 批量 INSERT（逐条检查）
    for (const auto &r: buffer) {
        const auto insert_res = databaseDB_2_con.Query(
            "INSERT INTO tmp_calc VALUES (?, ?, ?, ?) "
            "ON CONFLICT (id) DO NOTHING;",
            r.id, r.ivl, r.cost, r.recall
        );

        if (!insert_res || insert_res->HasError()) {
            databaseDB_2_con.Query("ROLLBACK");
            std::cerr << "INSERT tmp_calc 失败: "
                    << insert_res->GetError() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    // UPDATE FROM
    const auto update_res = databaseDB_2_con.Query(R"SQL(
        UPDATE courseComplier c
        SET
          ivl    = t.ivl,
          cost   = t.cost,
          recall = t.recall
        FROM tmp_calc t
        WHERE c.id = t.id;
    )SQL");

    if (!update_res || update_res->HasError()) {
        databaseDB_2_con.Query("ROLLBACK");
        std::cerr << "数据写回失败: "
                << update_res->GetError() << std::endl;
        std::exit(EXIT_FAILURE);
    }

    // commit
    const auto commit_res = databaseDB_2_con.Query("COMMIT");
    if (!commit_res || commit_res->HasError()) {
        std::cerr << "COMMIT 失败" << std::endl
                << commit_res->GetError();
        std::exit(EXIT_FAILURE);
    }
}

// 初次检查 user 表，合并 courseComplier 的数据
void DataInitializer::learningDataInitializer() {
    auto result = databaseDB_2_con.Query(R"SQL(
        INSERT INTO "history" (id, note, d1, h1, ivl, cost, recall)
        SELECT
            c.id,
            c.note,
            c.d0,
            c.h0,
            c.ivl,
            c.cost,
            c.recall
        FROM courseComplier c
        WHERE c.isLeaf = true
          AND NOT EXISTS (
              SELECT 1
              FROM "history" h
              WHERE h.id = c.id
          );
    )SQL"
    );
}

// 导出所有初始化成功的数据库表
bool DataInitializer::export_all_tables_to_csvs() {
    for (const auto &csv_file_path: DatabaseChecker::list_data_csv_files(GENERATED_FOLDER_PATH)) {
        std::string tableName = std::filesystem::path(csv_file_path).stem().stem().string();
        if (!export_table_to_csv(tableName, csv_file_path))
            throw std::runtime_error("导出所有初始化成功的数据库表失败！");
    }
    return true;
}

bool DataInitializer::export_table_to_csv(const std::string &table_name,
                                          const std::string &csv_path) {
    const std::string sql =
            "COPY " + table_name +
            " TO '" + csv_path +
            "' (HEADER, DELIMITER ',');";

    auto res = databaseDB_2_con.Query(sql);

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




// f(ivl, recall) -> card / ivl == 1 && recall 升序
// f(card.name) -> audio
// f(audio，card.name) -> o0[string “认识” "模糊" “不认识”]，trust[TO DO] / (output) 用户作答 得到包含模糊的结果和信任度
// f(o0, trust) -> o[bool] / 将包括模糊的结果编译成布尔值结果
// f(o, recall, d0, h0) -> d1, h1
// f(d1, h1) -> ivl, cost, recall / 由 d1, h1 连立查表近似得出
