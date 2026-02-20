//
// Created by opus arc on 2026/2/20.
//

#include "AozoraDatabase.h"

void aozora_DatabaseChecker();
void aozora_DataInitializer();
void aozora_MasterScheduler();

AozoraDatabase::AozoraDatabase() {
    try {
        aozora_DatabaseChecker();
        aozora_DataInitializer();
        aozora_MasterScheduler();
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

// 控制生命周期

void aozora_DatabaseChecker() {
    try {
        DatabaseChecker databaseChecker;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
void aozora_DataInitializer() {
    try {
        DataInitializer dataInitializer;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
void aozora_MasterScheduler() {
    try {
        MasterScheduler masterScheduler;
    } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
