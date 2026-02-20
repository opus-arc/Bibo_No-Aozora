//
// Created by opus arc on 2026/2/16.
//

#include "MyEssentia.h"

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

void MyEssentia::DumpPool(const Pool &pool) {
    const vector<string> names = pool.descriptorNames();
    cout << "===== Pool descriptors (" << names.size() << ") =====" << endl;

    for (const auto &key: names) {
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
        if (pool.contains<vector<Real> >(key)) {
            const auto &v = pool.value<vector<Real> >(key);
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
        if (pool.contains<vector<string> >(key)) {
            const auto &v = pool.value<vector<string> >(key);
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
        if (pool.contains<vector<vector<Real> > >(key)) {
            const auto &vv = pool.value<vector<vector<Real> > >(key);
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

MyEssentia::~MyEssentia() {
    essentia::shutdown();
}
