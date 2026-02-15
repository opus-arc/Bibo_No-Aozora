//
// Created by opus arc on 2026/2/11.
//

#include "Terminal.h"

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

#include "../backend/database/DatabaseChecker.h"
#include "../backend/mapper/DataInitializer.h"
#include "Terminal.h"

#define ICON_PATH "../icon/aozora.txt"


// 表单先建上
// DuckdbCompiler duckDBCompiler;

// demo 临时效果
// void demo() {
//     try {
//         // 临时清空这张表 非常危险
//         duckDBCompiler.databaseDB_con.Query(R"SQL(
//             TRUNCATE TABLE user;
//         )SQL");
//
//         // 初始化 user 表单
//         duckDBCompiler.demoCardsQuery();
//         duckDBCompiler.demoPlayLowestDifficultyUser();
//
//     } catch (const std::exception &e) {
//         std::cerr << e.what() << std::endl;
//         std::exit(EXIT_FAILURE);
//     }
//
// }

// Global state: whether demo is running
static bool g_demoRunning = false;

// Print terminal icon + header
void printTerminalHeader() {
    const std::string path = ICON_PATH;

    std::ifstream in(path, std::ios::in);
    if (!in) {
        std::cerr << "Failed to open file: " << path << "\n";
        return;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << '\n';
    }

    std::cout << "--++++----------------------+++++--\n";
    std::cout << "++----++++++++++++++++++++++-----++\n";
    std::cout << "       [VERSION: Demo_1.1]        \n";
    std::cout << "                                   \n";

    in.close();
}

static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));
}

static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

void printHelp() {
    std::cout
        << "\nAozora Terminal Commands\n"
        << "------------------------\n"
        << ":help   Show this help message\n"
        << ":start  Start demo session\n"
        << ":quit   Stop demo session\n"
        << ":exit   Exit the program\n"
        << "\nNotes\n"
        << "-----\n"
        << "• Any input line beginning with ':' is intercepted as a command.\n";
}

// Handle intercepted command. Return true if should exit.
bool handleCommand(std::string cmdLine) {
    // cmdLine includes leading ':'; remove it
    if (!cmdLine.empty() && cmdLine[0] == ':') cmdLine.erase(0, 1);
    trim(cmdLine);

    if (cmdLine == "help") {
        printHelp();
        return false;
    }

    if (cmdLine == "start") {
        if (g_demoRunning) {
            // std::cout << "Demo is already running.\n";

        } else {
            g_demoRunning = true;
            std::cout << "Demo started.\n";

            // demo();
            // duckDBCompiler.demoPlayLowestDifficultyUser();

            // TODO: call your own interface here
        }
        return false;
    }

    if (cmdLine == "quit") {
        if (!g_demoRunning) {
            std::cout << "Demo is not running.\n";
        } else {
            g_demoRunning = false;
            std::cout << "Demo stopped. \n";
        }
        return false;
    }

    if (cmdLine == "exit") {
        std::cout << "Goodbye. Shutting down Aozora Terminal...\n";
        return true;
    }

    if (cmdLine.empty()) {
        // User typed ":" only — treat as no-op or show hint
        std::cout << "(Hint: type :help for command list)\n";
        return false;
    }

    std::cout << "Unknown command: :" << cmdLine << "\n"
              << "Type :help to see available commands.\n";
    return false;
}

// Handle normal input (non-command). You can replace this logic freely.
void handleNormalInput(const std::string& line) {
    // For now: just echo. Replace with your own pipeline / interface.
    if (!g_demoRunning) {
        std::cout << "bibo: command not found: " << line << "\n";
    }else {

        // duckDBCompiler.demoUpdate(line);
        // duckDBCompiler.demoPlayLowestDifficultyUser();
    }
}

Terminal::Terminal() {

    printTerminalHeader();

    std::string line;
    while (true) {
        // Prompt changes based on demo state
        if (g_demoRunning) {
            std::cout << "Aozora Demo Running ~ % " << std::flush;
        } else {
            std::cout << "Aozora ~ % " << std::flush;
        }

        if (!std::getline(std::cin, line)) {
            // EOF / stream closed
            std::cout << "\nInput stream closed. Exiting...\n";
            break;
        }

        // Intercept command lines that start with ':'
        if (!line.empty() && line[0] == ':') {
            const bool shouldExit = handleCommand(line);
            if (shouldExit) break;
            continue;
        }

        // Normal input path
        handleNormalInput(line);
    }

}

// 此处用函数作用域手动析构 database_con
// 否则在下一层中会出现类似数据库锁死的 bug
void databaseChecker() {
    try {
        const DatabaseChecker databaseChecker;
    }catch (std::exception& e) {
        std::cout << e.what();
        std::exit(EXIT_FAILURE);
    }
}

void dataInitializer() {
    try {
        const DataInitializer dataInitializer;
    }catch (std::exception& e) {
        std::cout << e.what();
        std::exit(EXIT_FAILURE);
    }
}


