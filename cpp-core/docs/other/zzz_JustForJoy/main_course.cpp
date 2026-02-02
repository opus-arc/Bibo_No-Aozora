
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

constexpr double PI = 3.14159265358979323846;
constexpr int SAMPLE_RATE = 44100;

// ----------------- 基础工具：写 WAV 文件 -----------------

void writeWav(const std::string &filename,
              const std::vector<int16_t> &samples,
              int sampleRate = SAMPLE_RATE,
              int numChannels = 1) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }

    uint32_t dataSize = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    uint32_t fmtChunkSize = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t bitsPerSample = 16;
    uint16_t blockAlign = numChannels * bitsPerSample / 8;
    uint32_t byteRate = sampleRate * blockAlign;
    uint32_t chunkSize = 4 + (8 + fmtChunkSize) + (8 + dataSize);

    // RIFF header
    out.write("RIFF", 4);
    out.write(reinterpret_cast<const char *>(&chunkSize), 4);
    out.write("WAVE", 4);

    // fmt chunk
    out.write("fmt ", 4);
    out.write(reinterpret_cast<const char *>(&fmtChunkSize), 4);
    out.write(reinterpret_cast<const char *>(&audioFormat), 2);
    out.write(reinterpret_cast<const char *>(&numChannels), 2);
    out.write(reinterpret_cast<const char *>(&sampleRate), 4);
    out.write(reinterpret_cast<const char *>(&byteRate), 4);
    out.write(reinterpret_cast<const char *>(&blockAlign), 2);
    out.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

    // mapper chunk
    out.write("mapper", 4);
    out.write(reinterpret_cast<const char *>(&dataSize), 4);
    out.write(reinterpret_cast<const char *>(samples.data()), dataSize);

    out.close();
    std::cout << "Wrote " << filename << " (" << samples.size() << " samples)" << std::endl;
}

// ----------------- 频率 & 简易钢琴 inharmonicity -----------------

double midiToFreq(int midiNote, double a4 = 440.0) {
    return a4 * std::pow(2.0, (midiNote - 69) / 12.0);
}

// 钢琴式不等间距谐波：f_n = n f0 sqrt(1 + B n^2)
double pianoHarmonicFreq(double f0, int n, double B) {
    double nn = static_cast<double>(n);
    return nn * f0 * std::sqrt(1.0 + B * nn * nn);
}

// ----------------- 谐波 / 包络 模型 -----------------

struct Harmonic {
    int n; // 第 n 个谐波
    double relAmplitude; // 相对强度
    double extraDamp; // 额外阻尼（越大衰减越快）
    double detuneCents; // 轻微失谐（增加厚度）
};

// 简单 ADSR 包络（对整个声部）
double adsr_normal(
    const float t,
    const float totalDuration,
    const float attack,
    const float decay,
    const float sustainLevel,
    const float release
) {
    double sustainStart = attack + decay;
    double sustainEnd = totalDuration - release;

    if (t < 0.0) return 0.0;
    if (t < attack) {
        return t / attack;
    } else if (t < sustainStart) {
        double x = (t - attack) / decay;
        return 1.0 + (sustainLevel - 1.0) * x; // 线性从1到sustain
    } else if (t < sustainEnd) {
        return sustainLevel;
    } else if (t < totalDuration) {
        double x = (t - sustainEnd) / release;
        return sustainLevel * (1.0 - x); // 线性衰减到0
    } else {
        return 0.0;
    }
}

// 钢琴风格谐波合成：一个声部（单个 note）
double synthPianoVoice(const double t,
                       const double f0,
                       const std::vector<Harmonic> &harmonics,
                       const double B, // inharmonicity
                       const double globalDamp // 全局阻尼基准
) {
    double sample = 0.0;

    for (const auto &h: harmonics) {
        double n = static_cast<double>(h.n);
        // 基于钢琴弦的不等间距
        double fn = pianoHarmonicFreq(f0, h.n, B);

        // 轻微 detune（以 cents 为单位）
        double detuneFactor = std::pow(2.0, h.detuneCents / 1200.0);
        fn *= detuneFactor;

        double phase = 2.0 * PI * fn * t;

        // 额外阻尼：高次谐波更快消失
        double damp = std::exp(-(globalDamp + h.extraDamp) * t);

        sample += h.relAmplitude * damp * std::sin(phase);
    }

    return sample;
}

// ----------------- 生成一个轨道（多个 note，可叠加成和弦） -----------------

std::vector<int16_t> renderPiano(
    const std::vector<int> &midiNotes,
    double seconds,
    const std::vector<Harmonic> &harmonics,
    double B, // inharmonicity
    double globalDamp, // 基础阻尼
    double attack, double decay,
    double sustainLevel, double release
) {
    int totalSamples = static_cast<int>(seconds * SAMPLE_RATE);
    std::vector<int16_t> buffer(totalSamples, 0);

    for (int i = 0; i < totalSamples; ++i) {
        double t = static_cast<double>(i) / SAMPLE_RATE;

        // 全局 ADSR
        double env = adsr_normal(t, seconds, attack, decay, sustainLevel, release);

        double mix = 0.0;
        for (int note: midiNotes) {
            double f0 = midiToFreq(note);
            mix += synthPianoVoice(t, f0, harmonics, B, globalDamp);
        }

        // 简单防止爆音：和弦时稍微压缩一下
        mix *= 0.3;

        double s = env * mix;

        // 限幅到 16-bit
        if (s > 1.0) s = 1.0;
        if (s < -1.0) s = -1.0;

        buffer[i] = static_cast<int16_t>(s * 32767.0);
    }

    return buffer;
}

// ----------------- 主程序：三个例子 -----------------

int _main() {
    // 例子一：柔和钢琴 C4
    {
        std::vector<Harmonic> softHarmonics = {
            {1, 1.00, 1.0, 0.0}, // 基频，比较慢衰减
            {2, 0.60, 2.0, 1.0}, // 高次稍弱
            {3, 0.40, 3.0, -2.0},
            {4, 0.25, 4.0, 3.0},
            {5, 0.18, 5.0, -4.0},
            {6, 0.12, 6.0, 2.0},
        };

        double B = 2.0e-4; // 高音钢琴 inharmonicity 较小
        double globalDamp = 0.8; // 柔和一些，尾巴稍长
        double duration = 3.0; // 秒
        double attack = 0.01;
        double decay = 0.3;
        double sustainLvl = 0.6;
        double release = 0.8;

        auto buf = renderPiano({60}, duration, softHarmonics, B,
                               globalDamp, attack, decay, sustainLvl, release);
        writeWav("piano_soft_C4.wav", buf);
    }

    // 例子二：明亮钢琴 C4（更强的高次谐波、更快的高频衰减）
    {
        std::vector<Harmonic> brightHarmonics = {
            {1, 1.00, 0.8, 0.0},
            {2, 0.90, 1.5, 3.0},
            {3, 0.75, 2.5, -3.0},
            {4, 0.55, 3.5, 5.0},
            {5, 0.40, 4.5, -6.0},
            {6, 0.30, 5.5, 4.0},
            {7, 0.22, 6.5, -4.0},
            {8, 0.15, 7.5, 2.0},
        };

        double B = 3.0e-4; // 稍微再“硬”一点
        double globalDamp = 1.0; // 整体更干一点
        double duration = 2.5;
        double attack = 0.005; // 更快起音
        double decay = 0.25;
        double sustainLvl = 0.5;
        double release = 0.6;

        auto buf = renderPiano({60}, duration, brightHarmonics, B,
                               globalDamp, attack, decay, sustainLvl, release);
        writeWav("piano_bright_C4.wav", buf);
    }

    // 例子三：C 大三和弦（C4–E4–G4），用柔和参数
    {
        std::vector<Harmonic> chordHarmonics = {
            {1, 1.00, 1.0, 0.0},
            {2, 0.60, 2.0, 1.5},
            {3, 0.40, 3.0, -1.5},
            {4, 0.25, 4.0, 3.0},
            {5, 0.18, 5.0, -3.0},
            {6, 0.12, 6.0, 2.0},
        };

        double B = 2.0e-4;
        double globalDamp = 0.9;
        double duration = 3.5;
        double attack = 0.01;
        double decay = 0.35;
        double sustainLvl = 0.55;
        double release = 1.0;

        // C4, E4, G4
        std::vector<int> notes = {60, 64, 67};

        auto buf = renderPiano(notes, duration, chordHarmonics, B,
                               globalDamp, attack, decay, sustainLvl, release);
        writeWav("piano_chord_C_major.wav", buf);
    }

    return 0;
}
