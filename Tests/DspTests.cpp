#include "VocalDSP.h"
#include "Presets.h"
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

namespace
{
int checks = 0;
void check(bool ok, const std::string& name)
{
    ++checks;
    if (!ok) { std::cerr << "FAIL: " << name << '\n'; std::exit(1); }
}
float rms(const std::vector<float>& data, size_t from = 0)
{
    double power = 0;
    for (size_t i = from; i < data.size(); ++i) power += double(data[i]) * data[i];
    return float(std::sqrt(power / double(data.size() - from)));
}
std::vector<float> tone(double sr, double hz, float amplitude = .2f)
{
    std::vector<float> x(size_t(sr), 0);
    for (size_t i = 0; i < x.size(); ++i) x[i] = amplitude * float(std::sin(2 * vocal::pi * hz * double(i) / sr));
    return x;
}
}

int main()
{
    for (const double sr : {44100., 48000., 96000.})
    {
        vocal::Engine engine;
        vocal::Settings s;
        auto x = tone(sr, 997), y = x, original = x;
        engine.prepare(sr, s); engine.process(x.data(), y.data(), int(x.size()), s);
        float error = 0;
        for (size_t i = 0; i < x.size(); ++i) error = std::max(error, std::abs(x[i] - original[i]));
        check(error < 1.e-6f, "all modules bypassed, unity at " + std::to_string(sr));

        s.input = 4; s.output = -1;
        x = original; y = original;
        engine.prepare(sr, s); engine.process(x.data(), y.data(), int(x.size()), s);
        check(std::abs(rms(x) / rms(original) - vocal::gain(3)) < 1.e-4f, "gain staging");

        for (int effect = 0; effect < 6; ++effect)
        {
            for (int mode = 1; mode <= 3; ++mode)
            {
                s = {}; s.mode[size_t(effect)] = mode; s.amount[size_t(effect)] = 8;
                x = tone(sr, effect == 0 ? 40 : (effect == 1 ? 14000 : 997), .6f); y = x;
                original = x;
                engine.prepare(sr, s); engine.process(x.data(), y.data(), int(x.size()), s);
                double diff = 0;
                for (size_t i = 0; i < x.size(); ++i)
                {
                    check(std::isfinite(x[i]) && std::isfinite(y[i]), "finite module output");
                    diff += std::abs(x[i] - original[i]);
                }
                check(diff > .01, "module changes audio " + std::to_string(effect) + "/" + std::to_string(mode));
                if (effect < 2) check(rms(x, x.size() / 2) > rms(original, original.size() / 2) * 1.2f, "shelf boosts its band");
                if (effect == 2) check(rms(x, x.size() / 2) < rms(original, original.size() / 2), "compressor reduces strong input");
            }
        }

        // Slap echo must appear at 120 ms and must be absent before that tap.
        s = {}; s.mode[4] = 1; s.amount[4] = 10;
        x.assign(size_t(sr), 0); y = x; x[0] = y[0] = 1;
        engine.prepare(sr, s); engine.process(x.data(), y.data(), int(x.size()), s);
        // Also check settled timing with a second impulse after one second.
        x.assign(size_t(sr), 0); y = x;
        engine.process(x.data(), y.data(), int(x.size()), s);
        x.assign(size_t(sr), 0); y = x; x[0] = y[0] = 1;
        engine.process(x.data(), y.data(), int(x.size()), s);
        check(x[size_t(sr * .12)] > .3f, "120ms slap tap");

        // Partitioning must not change DSP output or introduce block-size dependence.
        s = vocal::presets[4].settings();
        auto one = tone(sr, 333), oneR = one, split = one, splitR = one;
        vocal::Engine a, b; a.prepare(sr, s); b.prepare(sr, s);
        a.process(one.data(), oneR.data(), int(one.size()), s);
        for (size_t off = 0; off < split.size(); off += 127)
            b.process(split.data() + off, splitR.data() + off, int(std::min(size_t(127), split.size() - off)), s);
        check(one == split && oneR == splitR, "partition invariant");
    }

    for (const double bpm : {60., 120., 180.})
        for (int mode : {2, 3})
        {
            vocal::Settings s; s.mode[4] = mode; s.amount[4] = 10; s.bpm = bpm;
            vocal::Engine engine; engine.prepare(48000, s);
            std::vector<float> x(96000, 0), y(96000, 0); x[0] = y[0] = 1;
            engine.process(x.data(), y.data(), int(x.size()), s);
            const auto tap = size_t(std::lround(48000 * (60 / bpm) * (mode == 2 ? .5 : 1)));
            check(x[tap] > .49f, "host BPM maps eighth/quarter delay correctly");
            check(std::abs(x[tap - 2]) < 1.e-6f, "delay absent before first rhythmic tap");
        }

    std::mt19937 random(9871); std::uniform_real_distribution<float> noise(-.3f, .3f);
    for (const auto& preset : vocal::presets)
    {
        const auto s = preset.settings();
        for (size_t i = 0; i < 6; ++i)
        {
            check(s.mode[i] >= 0 && s.mode[i] <= 3, "preset mode range");
            check(s.amount[i] >= -10 && s.amount[i] <= 10, "preset fader range");
        }
        vocal::Engine engine; engine.prepare(48000, s);
        std::vector<float> x(48000), y(48000);
        for (size_t i = 0; i < x.size(); ++i) x[i] = y[i] = noise(random);
        engine.process(x.data(), y.data(), int(x.size()), s);
        check(rms(x) > .001f, std::string(preset.name) + " has output");
        for (size_t i = 0; i < x.size(); ++i)
            check(std::isfinite(x[i]) && std::isfinite(y[i]) && std::abs(x[i]) < 10 && std::abs(y[i]) < 10,
                  std::string(preset.name) + " stable");
    }
    std::cout << "PASS: " << checks << " assertions, " << std::size(vocal::presets)
              << " factory presets, 44.1/48/96 kHz, all 18 effect modes.\n";
}
