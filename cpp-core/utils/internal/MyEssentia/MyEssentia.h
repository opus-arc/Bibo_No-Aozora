//
// Created by opus arc on 2026/2/16.
//

#ifndef BIBO_NO_AOZORA_ESSENTIA_H
#define BIBO_NO_AOZORA_ESSENTIA_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>

using namespace std;
using namespace essentia;
using namespace essentia::standard;

class MyEssentia {
public:
    MyEssentia();

    // 你要的“外包一层”：输入 filename，返回结果（Pool）。
    // Swift/SwiftUI 侧如果要更好桥接，可以再把 Pool 序列化成 JSON/键值对数组。
    static Pool musicExtractor(
        const string &audioName,
        int analysisSampleRate,
        int frameSize,
        int hopSize
    );

    // 提取“更高一层”的（但仍可解释的）中层特征：全部只用 Essentia 自带算法。
    // 说明：此函数只负责“算 + 写入 Pool”，打印仍可复用 DumpPool。
    // 你只需要在 myEssentia.h 里补充对应声明即可。
    static Pool midLevelExtractor(const std::string& audioName,
                                  int sampleRate,
                                  int tonalFrameSize,
                                  int tonalHopSize);

    // 将 Pool 中的所有 descriptor 做一个“尽可能通用”的字符串化输出。
    // 说明：Essentia 的 Pool 是强类型容器，不同 key 对应不同 value 类型。
    // 这里用 contains<T>/value<T> 逐类尝试，覆盖常见类型。
    static void DumpPool(const Pool &pool);

    void extractFolderToCsv(const std::string& inputDir, const std::string& outputDir) const;

    ~MyEssentia();
};


#endif //BIBO_NO_AOZORA_ESSENTIA_H
