//
// Created by opus arc on 2026/2/16.
//

#include "MyEssentia.h"
#include <filesystem>
#include <fstream>
#include <set>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

MyEssentia::MyEssentia() {
    essentia::init();
}

Pool MyEssentia::musicExtractor(const string &audioName,
                                const int analysisSampleRate = 44100,
                                const int frameSize = 2048,
                                const int hopSize = 512) {
    AlgorithmFactory &factory = AlgorithmFactory::instance();

    // 此版本的 MusicExtractor：输入端口 filename，输出端口 results/resultsFrames。
    Algorithm *extractor = essentia::standard::AlgorithmFactory::create("MusicExtractor",
                                                                        "analysisSampleRate", analysisSampleRate,
                                                                        "lowlevelFrameSize", frameSize,
                                                                        "lowlevelHopSize", hopSize);

    // 输入：文件名
    const string &fn = audioName;
    extractor->input("filename").set(fn);

    // 输出：必须绑定 resultsFrames，否则会抛异常
    Pool results;
    Pool resultsFrames;
    extractor->output("results").set(results);
    extractor->output("resultsFrames").set(resultsFrames);

    extractor->compute();

    delete extractor;

    // 目前只返回了聚合后的 results。
    // 如果需要逐帧特征，也可以把 resultsFrames 合并进 results 或改成返回二元组。
    return results;
}

// 提取“更高一层”的（但仍可解释的）中层特征：全部只用 Essentia 自带算法。
// 说明：此函数只负责“算 + 写入 Pool”，打印仍可复用 DumpPool。
// 你只需要在 myEssentia.h 里补充对应声明即可。
Pool MyEssentia::midLevelExtractor(const string &audioName,
                                   const int sampleRate = 44100,
                                   const int tonalFrameSize = 4096,
                                   const int tonalHopSize = 2048) {
    AlgorithmFactory &factory = AlgorithmFactory::instance();

    Pool out;

    // 1) 读取音频（单声道 + 重采样）
    vector<Real> audio;
    {
        Algorithm *loader = factory.create("MonoLoader",
                                           "filename", audioName,
                                           "sampleRate", sampleRate);
        loader->output("audio").set(audio);
        loader->compute();
        delete loader;
    }

    // ===== Loudness / Dynamics =====
    // DynamicComplexity（和你在 MusicExtractor 里看到的 lowlevel.dynamic_complexity 同宗，但这里直接算）
    {
        Algorithm *dyn = factory.create("DynamicComplexity",
                                        "sampleRate", (Real) sampleRate);
        Real dynamicComplexity = 0.0;
        Real loudnessDb = 0.0;

        dyn->input("signal").set(audio);
        dyn->output("dynamicComplexity").set(dynamicComplexity);
        dyn->output("loudness").set(loudnessDb);
        dyn->compute();
        delete dyn;

        out.add("midlevel.dynamic_complexity", dynamicComplexity);
        out.add("midlevel.loudness_db_estimate", loudnessDb);
    }

    // RhythmDescriptors：bpm、beats_position、histogram、peaks 等
    // 注意：std::RhythmDescriptors 在该版本中没有可配置参数（包括 sampleRate）。
    {
        Algorithm *rdesc = factory.create("RhythmDescriptors");

        Real bpm = 0.0;
        Real confidence = 0.0;
        vector<Real> beatsPosition;
        vector<Real> bpmEstimates;
        vector<Real> bpmIntervals;
        vector<Real> histogram;

        Real firstPeakBpm = 0.0, firstPeakWeight = 0.0, firstPeakSpread = 0.0;
        Real secondPeakBpm = 0.0, secondPeakWeight = 0.0, secondPeakSpread = 0.0;

        rdesc->input("signal").set(audio);

        // outputs: see https://essentia.upf.edu/reference/std_RhythmDescriptors.html
        rdesc->output("bpm").set(bpm);
        rdesc->output("confidence").set(confidence);
        rdesc->output("beats_position").set(beatsPosition);
        rdesc->output("bpm_estimates").set(bpmEstimates);
        rdesc->output("bpm_intervals").set(bpmIntervals);
        rdesc->output("histogram").set(histogram);

        rdesc->output("first_peak_bpm").set(firstPeakBpm);
        rdesc->output("first_peak_weight").set(firstPeakWeight);
        rdesc->output("first_peak_spread").set(firstPeakSpread);

        rdesc->output("second_peak_bpm").set(secondPeakBpm);
        rdesc->output("second_peak_weight").set(secondPeakWeight);
        rdesc->output("second_peak_spread").set(secondPeakSpread);

        rdesc->compute();
        delete rdesc;

        out.add("midlevel.rhythm.bpm", bpm);
        out.add("midlevel.rhythm.confidence", confidence);
        out.add("midlevel.rhythm.beats_position", beatsPosition);
        out.add("midlevel.rhythm.bpm_estimates", bpmEstimates);
        out.add("midlevel.rhythm.bpm_intervals", bpmIntervals);
        out.add("midlevel.rhythm.bpm_histogram", histogram);

        out.add("midlevel.rhythm.bpm_histogram_first_peak_bpm", firstPeakBpm);
        out.add("midlevel.rhythm.bpm_histogram_first_peak_weight", firstPeakWeight);
        out.add("midlevel.rhythm.bpm_histogram_first_peak_spread", firstPeakSpread);

        out.add("midlevel.rhythm.bpm_histogram_second_peak_bpm", secondPeakBpm);
        out.add("midlevel.rhythm.bpm_histogram_second_peak_weight", secondPeakWeight);
        out.add("midlevel.rhythm.bpm_histogram_second_peak_spread", secondPeakSpread);
    }

    // Danceability：基于 DFA 的舞动性度量（非“mood”那类黑箱标签，仍属可解释的中层节奏量）
    {
        // 该算法既有 streaming 也有 standard 版本，这里使用 standard 版本
        Algorithm *dance = factory.create("Danceability",
                                          "sampleRate", (Real) sampleRate);
        Real danceability = 0.0;
        vector<Real> dfa;

        dance->input("signal").set(audio);
        dance->output("danceability").set(danceability);
        dance->output("dfa").set(dfa);
        dance->compute();
        delete dance;

        out.add("midlevel.rhythm.danceability", danceability);
        out.add("midlevel.rhythm.dfa", dfa);
    }

    // ===== Tonal / Harmony =====
    // TonalExtractor：和弦序列/和弦强度/HPCP/key 等（非常适合你后续构造“和声复杂度”一类指标）
    {
        Algorithm *tonal = factory.create("TonalExtractor",
                                          "frameSize", tonalFrameSize,
                                          "hopSize", tonalHopSize,
                                          "tuningFrequency", 440.0);

        Real chordsChangesRate = 0.0;
        vector<Real> chordsHistogram;
        string chordsKey;
        Real chordsNumberRate = 0.0;
        vector<string> chordsProgression;
        string chordsScale;
        vector<Real> chordsStrength;
        vector<vector<Real> > hpcp;
        vector<vector<Real> > hpcpHighres;
        string keyKey;
        string keyScale;
        Real keyStrength = 0.0;

        tonal->input("signal").set(audio);
        tonal->output("chords_changes_rate").set(chordsChangesRate);
        tonal->output("chords_histogram").set(chordsHistogram);
        tonal->output("chords_key").set(chordsKey);
        tonal->output("chords_number_rate").set(chordsNumberRate);
        tonal->output("chords_progression").set(chordsProgression);
        tonal->output("chords_scale").set(chordsScale);
        tonal->output("chords_strength").set(chordsStrength);
        tonal->output("hpcp").set(hpcp);
        tonal->output("hpcp_highres").set(hpcpHighres);
        tonal->output("key_key").set(keyKey);
        tonal->output("key_scale").set(keyScale);
        tonal->output("key_strength").set(keyStrength);

        tonal->compute();
        delete tonal;

        out.add("midlevel.tonal.chords_changes_rate", chordsChangesRate);
        out.add("midlevel.tonal.chords_histogram", chordsHistogram);
        out.add("midlevel.tonal.chords_key", chordsKey);
        out.add("midlevel.tonal.chords_number_rate", chordsNumberRate);
        out.add("midlevel.tonal.chords_progression", chordsProgression);
        out.add("midlevel.tonal.chords_scale", chordsScale);
        out.add("midlevel.tonal.chords_strength", chordsStrength);
        // out.add("midlevel.tonal.hpcp", hpcp);
        // out.add("midlevel.tonal.hpcp_highres", hpcpHighres);
        out.add("midlevel.tonal.key", keyKey);
        out.add("midlevel.tonal.scale", keyScale);
        out.add("midlevel.tonal.key_strength", keyStrength);
    }

    return out;
}

namespace {
    // CSV escaping: wrap in quotes if needed, and double internal quotes.
    std::string csvEscape(const std::string& s) {
        bool needQuotes = false;
        for (char c : s) {
            if (c == ',' || c == '"' || c == '\n' || c == '\r') { needQuotes = true; break; }
        }
        if (!needQuotes) return s;

        std::string out;
        out.reserve(s.size() + 2);
        out.push_back('"');
        for (char c : s) {
            if (c == '"') out += "\"\"";
            else out.push_back(c);
        }
        out.push_back('"');
        return out;
    }

    template <typename T>
    std::string joinVector(const std::vector<T>& v) {
        std::ostringstream oss;
        // Use a stable, locale-independent formatting.
        oss.setf(std::ios::fixed);
        oss << std::setprecision(8);

        for (size_t i = 0; i < v.size(); ++i) {
            if (i) oss << ';';
            oss << v[i];
        }
        return oss.str();
    }

    // Convert a Pool value under `key` into a single CSV cell string.
    // Strategy:
    // - Real / string => scalar
    // - vector<Real> / vector<string> => ';' separated (quoted if CSV needs)
    // - vector<vector<Real>> => full serialization separated by '|' and ';'
    std::string poolCellString(const Pool& pool, const std::string& key) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss << std::setprecision(8);

        if (pool.contains<Real>(key)) {
            oss << pool.value<Real>(key);
            return oss.str();
        }
        if (pool.contains<std::string>(key)) {
            return pool.value<std::string>(key);
        }
        if (pool.contains<std::vector<Real>>(key)) {
            const auto& v = pool.value<std::vector<Real>>(key);
            return joinVector(v);
        }
        if (pool.contains<std::vector<std::string>>(key)) {
            const auto& v = pool.value<std::vector<std::string>>(key);
            std::ostringstream s;
            for (size_t i = 0; i < v.size(); ++i) {
                if (i) s << ';';
                s << v[i];
            }
            return s.str();
        }
        if (pool.contains<std::vector<std::vector<Real>>>(key)) {
            const auto& vv = pool.value<std::vector<std::vector<Real>>>(key);
            std::ostringstream s;
            s.setf(std::ios::fixed);
            s << std::setprecision(8);
            for (size_t fi = 0; fi < vv.size(); ++fi) {
                if (fi) s << '|';
                const auto& frame = vv[fi];
                for (size_t j = 0; j < frame.size(); ++j) {
                    if (j) s << ';';
                    s << frame[j];
                }
            }
            return s.str();
        }

        return ""; // unhandled type => empty cell
    }

    std::string sanitizeForFilename(std::string s) {
        // Keep it conservative: letters/digits/._- ; everything else becomes '_'
        for (char& c : s) {
            const unsigned char uc = static_cast<unsigned char>(c);
            if (std::isalnum(uc) || c == '.' || c == '_' || c == '-') continue;
            c = '_';
        }
        // Avoid empty names
        if (s.empty()) s = "unnamed";
        return s;
    }

    // Returns a Chinese comment/annotation for a given descriptor key.
    std::string zhCommentForKey(const std::string& key) {
        // 1) Exact matches for our midlevel keys (highest quality).
        static const std::unordered_map<std::string, std::string> exact = {
            {"midlevel.dynamic_complexity", "动态复杂度（响度随时间波动的复杂程度）"},
            {"midlevel.loudness_db_estimate", "响度估计（dB，算法内部估计值）"},

            {"midlevel.rhythm.bpm", "节拍速度（BPM）"},
            {"midlevel.rhythm.confidence", "BPM 置信度"},
            {"midlevel.rhythm.beats_position", "节拍位置序列（秒）"},
            {"midlevel.rhythm.bpm_estimates", "BPM 候选估计序列"},
            {"midlevel.rhythm.bpm_intervals", "BPM 间隔序列"},
            {"midlevel.rhythm.bpm_histogram", "BPM 直方图"},
            {"midlevel.rhythm.bpm_histogram_first_peak_bpm", "BPM 直方图第一峰值位置（BPM）"},
            {"midlevel.rhythm.bpm_histogram_first_peak_weight", "BPM 直方图第一峰值权重"},
            {"midlevel.rhythm.bpm_histogram_first_peak_spread", "BPM 直方图第一峰值扩散度"},
            {"midlevel.rhythm.bpm_histogram_second_peak_bpm", "BPM 直方图第二峰值位置（BPM）"},
            {"midlevel.rhythm.bpm_histogram_second_peak_weight", "BPM 直方图第二峰值权重"},
            {"midlevel.rhythm.bpm_histogram_second_peak_spread", "BPM 直方图第二峰值扩散度"},
            {"midlevel.rhythm.danceability", "舞动性（Danceability，基于 DFA 的节奏稳定性度量）"},
            {"midlevel.rhythm.dfa", "DFA 序列（用于舞动性计算的分形/标度特征）"},

            {"midlevel.tonal.chords_changes_rate", "和弦变化率（单位时间内和弦变化的频繁程度）"},
            {"midlevel.tonal.chords_histogram", "和弦直方图（和弦类别分布）"},
            {"midlevel.tonal.chords_key", "和弦主调（chords_key）"},
            {"midlevel.tonal.chords_number_rate", "和弦数量率（不同和弦出现的相对速率）"},
            {"midlevel.tonal.chords_progression", "和弦进行序列（按时间顺序）"},
            {"midlevel.tonal.chords_scale", "和弦音阶类型（大/小等，chords_scale）"},
            {"midlevel.tonal.chords_strength", "和弦置信强度序列（每个时间帧）"},
            {"midlevel.tonal.key", "调性主音（key_key）"},
            {"midlevel.tonal.scale", "调式（大调/小调，key_scale）"},
            {"midlevel.tonal.key_strength", "调性强度（key_strength）"},
        };
        auto it = exact.find(key);
        if (it != exact.end()) return it->second;

        // 2) Heuristic mapping for Essentia MusicExtractor keys.
        //    We do not attempt a perfect dictionary for all ~600 descriptors.
        //    Instead we translate common namespaces/tokens and keep the remainder readable.
        struct Token { const char* en; const char* zh; };
        static const Token tokens[] = {
            {"lowlevel", "低层"},
            {"midlevel", "中层"},
            {"highlevel", "高层"},
            {"rhythm", "节奏"},
            {"tonal", "调性/和声"},
            {"metadata", "元数据"},
            {"analysis", "分析"},

            {"bpm", "BPM"},
            {"loudness", "响度"},
            {"dynamic", "动态"},
            {"complexity", "复杂度"},
            {"spectral", "频谱"},
            {"mfcc", "MFCC"},
            {"hpcp", "HPCP（音高类别谱）"},
            {"key", "主音"},
            {"scale", "调式"},
            {"chords", "和弦"},
            {"histogram", "直方图"},
            {"centroid", "质心"},
            {"spread", "扩散度"},
            {"skewness", "偏度"},
            {"kurtosis", "峰度"},
            {"flatness", "平坦度"},
            {"rolloff", "滚降点"},
            {"flux", "通量"},
            {"energy", "能量"},
            {"zcr", "过零率"},
            {"pitch", "音高"},
            {"inharmonicity", "非谐性"},
            {"dissonance", "不协和度"},
            {"entropy", "熵"},
            {"onset", "起音"},
            {"beats", "节拍"},
            {"position", "位置"},
            {"confidence", "置信度"},
            {"rate", "比率"},
            {"mean", "均值"},
            {"var", "方差"},
            {"variance", "方差"},
            {"stdev", "标准差"},
            {"std", "标准差"},
            {"min", "最小值"},
            {"max", "最大值"},
        };

        // Split by '.' then '_' and translate known tokens.
        auto translateToken = [&](std::string t) -> std::string {
            // Lowercase copy for matching
            std::string low;
            low.reserve(t.size());
            for (char c : t) low.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            for (const auto& kv : tokens) {
                if (low == kv.en) return kv.zh;
            }
            // Keep original if unknown
            return t;
        };

        std::ostringstream oss;
        oss << "（";
        bool first = true;

        auto flushPiece = [&](const std::string& piece) {
            if (piece.empty()) return;
            // Further split by '_' for readability.
            std::string buf;
            for (char c : piece) {
                if (c == '_') {
                    if (!buf.empty()) {
                        if (!first) oss << "·";
                        oss << translateToken(buf);
                        first = false;
                        buf.clear();
                    }
                } else {
                    buf.push_back(c);
                }
            }
            if (!buf.empty()) {
                if (!first) oss << "·";
                oss << translateToken(buf);
                first = false;
            }
        };

        std::string piece;
        for (char c : key) {
            if (c == '.') {
                flushPiece(piece);
                piece.clear();
            } else {
                piece.push_back(c);
            }
        }
        flushPiece(piece);

        oss << "）";
        return oss.str();
    }

    void writePoolKeyValueCsv(const std::filesystem::path& csvPath, const Pool& pool) {
        std::ofstream ofs(csvPath);
        if (!ofs.is_open()) {
            throw std::runtime_error("Failed to open CSV for writing: " + csvPath.string());
        }

        // Header
        ofs << "key,value,comment_zh\n";

        // Stable ordering
        std::vector<std::string> names = pool.descriptorNames();
        std::sort(names.begin(), names.end());

        for (const auto& key : names) {
            std::string value = poolCellString(pool, key);
            std::string comment = zhCommentForKey(key);
            ofs << csvEscape(key) << "," << csvEscape(value) << "," << csvEscape(comment) << "\n";
        }
    }
} // namespace

// New API: batch-extract a folder to two CSVs (basic + Intermediate).
// - inputDir: folder containing audio files
// - outputDir: folder to place `basic-<stem>.csv` and `Intermediate-<stem>.csv`
void MyEssentia::extractFolderToCsv(const std::string& inputDir, const std::string& outputDir) const {
    namespace fs = std::filesystem;

    const fs::path inPath(inputDir);
    fs::path outPath(outputDir);
    // Normalize path to make the safety check robust:
    // - handle trailing separators (e.g. "results/") where filename() can be empty on some libc++ builds
    // - collapse ".." and "." segments lexically
    outPath = outPath.lexically_normal();
    if (outPath.filename().empty()) {
        outPath = outPath.parent_path();
    }

    if (!fs::exists(inPath) || !fs::is_directory(inPath)) {
        throw std::runtime_error("Input path is not a directory: " + inPath.string());
    }

    // Safety: only allow clearing a folder explicitly named "results".
    if (outPath.filename() != "results") {
        throw std::runtime_error(
            "Refusing to clear output directory because its last path component is not 'results': " + outPath.string());
    }

    if (!fs::exists(outPath)) {
        fs::create_directories(outPath);
    } else if (!fs::is_directory(outPath)) {
        throw std::runtime_error("Output path is not a directory: " + outPath.string());
    }

    // Clear output directory contents.
    for (const auto& entry : fs::directory_iterator(outPath)) {
        fs::remove_all(entry.path());
    }

    // Collect candidate audio files (common extensions). Extend as needed.
    std::vector<fs::path> audioFiles;
    for (const auto& entry : fs::directory_iterator(inPath)) {
        if (!entry.is_regular_file()) continue;
        const auto ext = entry.path().extension().string();
        std::string e;
        e.reserve(ext.size());
        for (char c : ext) e.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));

        if (e == ".wav" || e == ".mp3" || e == ".flac" || e == ".ogg" || e == ".m4a" || e == ".aiff" || e == ".aif") {
            audioFiles.push_back(entry.path());
        }
    }

    // Stable order
    std::sort(audioFiles.begin(), audioFiles.end());

    for (const auto& f : audioFiles) {
        const std::string stem = sanitizeForFilename(f.stem().string());

        // One audio -> two CSVs (do NOT merge pools)
        const Pool basic = this->musicExtractor(f.string());
        const Pool mid = this->midLevelExtractor(f.string());

        writePoolKeyValueCsv(outPath / ("basic-" + stem + ".csv"), basic);
        writePoolKeyValueCsv(outPath / ("Intermediate-" + stem + ".csv"), mid);
    }
}

void MyEssentia::DumpPool(const Pool &pool) {
    // CSV output to stdout: header + one row. Useful for quick inspection.
    // Note: For batch export, prefer `extractFolderToCsv()`.
    const auto names = pool.descriptorNames();

    // Header
    std::cout << "key,value,comment_zh" << std::endl;

    for (const auto &key: names) {
        std::string value = poolCellString(pool, key);
        std::string comment = zhCommentForKey(key);
        std::cout << csvEscape(key) << "," << csvEscape(value) << "," << csvEscape(comment) << std::endl;
    }
}

MyEssentia::~MyEssentia() {
    essentia::shutdown();
}
