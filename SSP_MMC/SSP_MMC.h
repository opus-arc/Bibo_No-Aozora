//
// Created by opus arc on 2026/1/21.
//

#ifndef BIBO_NO_AOZORA_SSP_MMC_H
#define BIBO_NO_AOZORA_SSP_MMC_H

#include <fstream>

class SSP_MMC {
    public:
    // 查 SSPMMC 调度算法结果表
    static double read_SSPMMC_Result(const std::string& type, const std::string& difficulty, double memoryState);
};


#endif //BIBO_NO_AOZORA_SSP_MMC_H