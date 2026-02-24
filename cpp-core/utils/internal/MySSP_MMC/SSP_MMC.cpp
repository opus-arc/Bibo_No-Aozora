//
// Created by opus arc on 2026/1/21.
//

#include "SSP_MMC.h"
#include <iostream>
#include <sstream>
#include <string>

#define RESULT_PATH "../utils/external/SSP-MMC/result/"

double SSP_MMC::read_SSPMMC_Result(const std::string& type, const std::string& difficulty, const double h) {
    // 拼接逻辑连接
    const std::string csvPath = RESULT_PATH + type + "-" + difficulty + ".csv";
    // 先构造
    std::ifstream csv;
    // 再打开
    csv.open(csvPath);
    // 检查状态
    if (!csv.is_open()) {
        // 终止当前函数的执行，沿栈往外找 catch 当异常没有办法被接住时，程序才会意外终止
        throw std::runtime_error("Can't open file: " + csvPath);
    }
    // 使用文件
    double data = -5.0; // 非法兜底
    std::string line;
    while (std::getline(csv, line)) {

        // 方案1：
        // const size_t pos = line.find(',');
        // double memoryState = std::stod(line.substr(0, pos));
        // double mapper = std::stod(line.substr(pos + 1));

        // 方案2:
        // double memoryState, mapper;
        // sscanf(line.c_str(), "%lf,%lf", &memoryState, &mapper);

        // 方案3:
        double _h, _data;
        std::istringstream iss(line);
        if (char comma = 0; !(iss >> _h >> comma >> _data) || comma != ',') {
            throw std::runtime_error("Invalid mapper line: " + line);
        }

        // -------------------------------

        // 隐匿函数声明，不干扰外部命名空间，不暴露接口
        // “ == ” 的本质是对比二进制数据，但是浮点数的二进制数据不尽然相同，需要用容差法进行数值上的精确比对
        // auto double_type_almost_equal = [](const double a, const double b, const double eps = 1e-9) {
        //     // |a - b| <= eps* (1.0 + max(|a|, |b|))
        //     // 这里加 1 是为了避免在 a b 都接近 0 的时候都被判断为不接近
        //     return std::fabs(a - b) <= eps * (1.0 + std::max(std::fabs(a), std::fabs(b)));
        // };

        // 但这里为了更兼容表本身的局限性，只做到了粗略的近似
        if (_h >= h) {
            data = _data;
            break;
        }
    }
    // 收回兜底
    if (data < 0) {
        throw std::runtime_error("Invalid mapper: " + std::to_string(data));
    }
    return data;
}

