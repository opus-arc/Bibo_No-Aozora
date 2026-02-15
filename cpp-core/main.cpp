#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>

#define AUDIO_PATH "../docs/other/testAudio/Waltz for Debby.m4a"

using namespace std;
using namespace essentia;
using namespace essentia::standard;

// 将 Pool 中的所有 descriptor 做一个“尽可能通用”的字符串化输出。
// 说明：Essentia 的 Pool 是强类型容器，不同 key 对应不同 value 类型。
// 这里用 contains<T>/value<T> 逐类尝试，覆盖常见类型。
static void DumpPool(const Pool& pool) {
    const vector<string> names = pool.descriptorNames();
    cout << "===== Pool descriptors (" << names.size() << ") =====" << endl;

    for (const auto& key : names) {
        // 1) 单值 Real
        if (pool.contains<Real>(key)) {
            cout << key << " = " << pool.value<Real>(key) << endl;
            continue;
        }

        // 2) 单值 string
        if (pool.contains<string>(key)) {
            cout << key << " = \"" << pool.value<string>(key) << "\"" << endl;
            continue;
        }

        // 3) vector<Real>
        if (pool.contains<vector<Real>>(key)) {
            const auto& v = pool.value<vector<Real>>(key);
            cout << key << " = vector<Real>(" << v.size() << ")";
            if (!v.empty()) {
                cout << " [";
                const size_t n = min<size_t>(v.size(), 8);
                for (size_t i = 0; i < n; ++i) {
                    if (i) cout << ", ";
                    cout << v[i];
                }
                if (v.size() > n) cout << ", ...";
                cout << "]";
            }
            cout << endl;
            continue;
        }

        // 4) vector<string>
        if (pool.contains<vector<string>>(key)) {
            const auto& v = pool.value<vector<string>>(key);
            cout << key << " = vector<string>(" << v.size() << ")";
            if (!v.empty()) {
                cout << " [\"" << v[0] << "\"";
                if (v.size() > 1) cout << ", ...";
                cout << "]";
            }
            cout << endl;
            continue;
        }

        // 5) vector<vector<Real>>（逐帧/序列特征很常见）
        if (pool.contains<vector<vector<Real>>>(key)) {
            const auto& vv = pool.value<vector<vector<Real>>>(key);
            cout << key << " = vector<vector<Real>>(" << vv.size() << ")";
            if (!vv.empty()) {
                cout << " firstFrameSize=" << vv[0].size();
            }
            cout << endl;
            continue;
        }

        // 覆盖不到的类型就只打印 key（避免崩溃）
        cout << key << " = <unhandled type>" << endl;
    }
}

// 你要的“外包一层”：输入 filename，返回结果（Pool）。
// Swift/SwiftUI 侧如果要更好桥接，可以再把 Pool 序列化成 JSON/键值对数组。
static Pool ExtractFeaturesFromFile(const string& filename,
                                   int analysisSampleRate = 44100,
                                   int frameSize = 2048,
                                   int hopSize = 512) {
    AlgorithmFactory& factory = AlgorithmFactory::instance();

    // 此版本的 MusicExtractor：输入端口 filename，输出端口 results/resultsFrames。
    Algorithm* extractor = factory.create("MusicExtractor",
                                          "analysisSampleRate", analysisSampleRate,
                                          "lowlevelFrameSize", frameSize,
                                          "lowlevelHopSize", hopSize);

    // 输入：文件名
    string fn = filename;
    extractor->input("filename").set(fn);

    // 输出：必须绑定 resultsFrames，否则会抛异常
    Pool results;
    Pool resultsFrames;
    extractor->output("results").set(results);
    extractor->output("resultsFrames").set(resultsFrames);

    extractor->compute();

    delete extractor;

    // 目前我们只返回聚合后的 results。
    // 如果你需要逐帧特征，也可以把 resultsFrames 合并进 results 或改成返回二元组。
    return results;
}

int main() {
    essentia::init();

    try {
        const string filename = AUDIO_PATH;

        Pool results = ExtractFeaturesFromFile(filename);

        cout << "===== Music Analysis Result (selected) =====" << endl;
        if (results.contains<Real>("rhythm.bpm")) {
            cout << "BPM: " << results.value<Real>("rhythm.bpm") << endl;
        }
        if (results.contains<string>("tonal.key_key") && results.contains<string>("tonal.key_scale")) {
            cout << "Key: "
                 << results.value<string>("tonal.key_key") << " "
                 << results.value<string>("tonal.key_scale") << endl;
        }

        // 打印“所有能打印的”结果 key/value
        DumpPool(results);
    }
    catch (const exception& e) {
        cerr << "Essentia error: " << e.what() << endl;
    }

    essentia::shutdown();
    return 0;
}