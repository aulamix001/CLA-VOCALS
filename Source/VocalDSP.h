#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace vocal
{
constexpr double pi = 3.14159265358979323846;
inline float gain(float db) { return std::pow(10.0f, db / 20.0f); }
inline float bounded(float v, float lo, float hi) { return std::clamp(v, lo, hi); }

struct Settings
{
    float input = 0, output = 0;
    std::array<float, 6> amount{}; // -10 .. +10, matching the manual's faders
    std::array<int, 6> mode{};     // 0 bypass, 1 lower row, 2 middle, 3 upper
    double bpm = 120;
};

struct Smoother
{
    float value = 0, target = 0;
    double current = 0, pole = 0;
    void prepare(double sr, float initial) { pole = std::exp(-1.0 / (sr * .02)); current = value = target = initial; }
    float next() { current = double(target) + pole * (current - double(target)); return value = float(current); }
};

struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    void shelf(double sr, double hz, float db, bool high)
    {
        const double a = std::pow(10.0, db / 40.0);
        const double w = 2 * pi * std::min(hz, sr * .45) / sr;
        const double c = std::cos(w), s = std::sin(w);
        const double beta = std::sqrt(2 * a) * s;
        double d;
        if (high)
        {
            b0 = a * ((a + 1) + (a - 1) * c + beta);
            b1 = -2 * a * ((a - 1) + (a + 1) * c);
            b2 = a * ((a + 1) + (a - 1) * c - beta);
            d = (a + 1) - (a - 1) * c + beta;
            a1 = 2 * ((a - 1) - (a + 1) * c);
            a2 = (a + 1) - (a - 1) * c - beta;
        }
        else
        {
            b0 = a * ((a + 1) - (a - 1) * c + beta);
            b1 = 2 * a * ((a - 1) - (a + 1) * c);
            b2 = a * ((a + 1) - (a - 1) * c - beta);
            d = (a + 1) + (a - 1) * c + beta;
            a1 = -2 * ((a - 1) + (a + 1) * c);
            a2 = (a + 1) + (a - 1) * c - beta;
        }
        b0 /= d; b1 /= d; b2 /= d; a1 /= d; a2 /= d;
    }
    float tick(float x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return float(y);
    }
};

class DelayLine
{
public:
    void prepare(int size) { data.assign(size_t(std::max(4, size)), 0); index = 0; }
    float read(float samples) const
    {
        float pos = float(index) - bounded(samples, 1, float(data.size() - 2));
        if (pos < 0) pos += float(data.size());
        const auto a = size_t(pos);
        const auto b = (a + 1) % data.size();
        return data[a] + (pos - float(a)) * (data[b] - data[a]);
    }
    void push(float x) { data[index] = x; index = (index + 1) % data.size(); }
private:
    std::vector<float> data;
    size_t index = 0;
};

class Engine
{
public:
    void prepare(double sampleRate, const Settings& initial = {})
    {
        sr = sampleRate;
        for (int i = 0; i < 6; ++i)
        {
            amount[size_t(i)].prepare(sr, initial.amount[size_t(i)]);
            enabled[size_t(i)].prepare(sr, initial.mode[size_t(i)] != 0 ? 1.f : 0.f);
        }
        input.prepare(sr, initial.input); output.prepare(sr, initial.output);
        for (int c = 0; c < 2; ++c)
        {
            low[size_t(c)].reset(); high[size_t(c)].reset();
            echo[size_t(c)].prepare(int(sr * 2.1));
            pitch[size_t(c)].prepare(int(sr * .12));
            for (auto& d : room[size_t(c)]) d.prepare(int(sr * .2));
            damping[size_t(c)].fill(0);
        }
        env = 0; phase = 0; coefficientCounter = 0;
        const auto m = std::clamp(initial.mode[4], 1, 3);
        const auto bpm = std::isfinite(initial.bpm) ? std::clamp(initial.bpm, 30.0, 300.0) : 120.0;
        delayTime.prepare(sr, float(sr * (m == 1 ? .12 : 60.0 / bpm * (m == 2 ? .5 : 1.0))));
    }

    // In-place stereo. Storage is allocated exclusively by prepare().
    void process(float* left, float* right, int count, const Settings& s)
    {
        input.target = bounded(s.input, -10, 10); output.target = bounded(s.output, -10, 10);
        for (size_t k = 0; k < 6; ++k)
        {
            amount[k].target = bounded(s.amount[k], -10, 10);
            enabled[k].target = s.mode[k] != 0 ? 1.f : 0.f;
        }
        const auto mode = [&s](size_t k) { return std::clamp(s.mode[k], 1, 3) - 1; };
        const double bpm = std::isfinite(s.bpm) ? std::clamp(s.bpm, 30.0, 300.0) : 120.0;
        delayTime.target = float(sr * (mode(4) == 0 ? .12 : (60.0 / bpm) * (mode(4) == 1 ? .5 : 1.0)));
        for (int n = 0; n < count; ++n)
        {
            std::array<float, 6> v{}, on{};
            for (size_t k = 0; k < 6; ++k) { v[k] = amount[k].next(); on[k] = enabled[k].next(); }
            if ((coefficientCounter++ & 31) == 0)
            {
                constexpr double bassHz[] {80, 180, 350}, trebleHz[] {2500, 6000, 10000};
                for (size_t c = 0; c < 2; ++c)
                {
                    low[c].shelf(sr, bassHz[mode(0)], v[0] * on[0], false);
                    high[c].shelf(sr, trebleHz[mode(1)], v[1] * on[1], true);
                }
            }
            std::array<float, 2> x {left[n] * gain(input.next()), right[n]};
            // Use the same gain on both channels (one smoother advance per frame).
            x[1] *= gain(input.value);
            for (size_t c = 0; c < 2; ++c) x[c] = high[c].tick(low[c].tick(x[c]));
            const float detector = std::max(std::abs(x[0]), std::abs(x[1]));
            constexpr float attack[] {.02f, .006f, .001f}, release[] {.20f, .09f, .045f};
            const float pole = std::exp(-1.0f / (float(sr) * (detector > env ? attack[mode(2)] : release[mode(2)])));
            env = detector + pole * (env - detector);
            const float strength = (v[2] + 10) * .05f;
            const float threshold = -8 - strength * 24;
            constexpr float ratio[] {2.f, 4.f, 10.f};
            const float reduction = std::max(0.f, 20 * std::log10(std::max(1.e-9f, env)) - threshold)
                                    * (1 - 1 / ratio[mode(2)]);
            const float compression = gain((-reduction + strength * 6) * on[2]);
            for (auto& y : x) y *= compression;

            const float dt = delayTime.next();
            const float echoWet = (v[4] + 10) * .025f * on[4];
            const float roomWet = (v[3] + 10) * .025f * on[3];
            const float pitchWet = (v[5] + 10) * .025f * on[5];
            const float out = gain(output.next());
            const float cents = float(mode(5) + 1) * 5;
            const float delta = std::pow(2.f, cents / 1200.f) - 1;
            phase += delta / float(sr * .03);
            if (phase >= 1) phase -= 1;
            std::array<float, 2> result{};
            for (size_t c = 0; c < 2; ++c)
            {
                // Four damped combs per channel; independent taps create a stereo field.
                constexpr float taps[] {.0297f, .0371f, .0411f, .0437f};
                float reverb = 0;
                const float size = 1 + float(mode(3)) * .65f;
                const float rt60 = .6f + float(mode(3)) * 1.1f;
                for (size_t t = 0; t < 4; ++t)
                {
                    const float seconds = taps[t] * size + float(c) * .0013f;
                    const float delayed = room[c][t].read(seconds * float(sr));
                    damping[c][t] += .35f * (delayed - damping[c][t]);
                    const float feedback = std::pow(.001f, seconds / rt60);
                    room[c][t].push(x[c] * on[3] + damping[c][t] * feedback);
                    reverb += delayed * .25f;
                }
                const float delayed = echo[c].read(dt * (c == 0 ? 1.f : 1.013f));
                echo[c].push(x[c] * on[4] + delayed * .32f);
                pitch[c].push(x[c] * on[5]);
                const float p = c == 0 ? phase : 1 - phase;
                const float q = p >= .5f ? p - .5f : p + .5f;
                const float w = .5f - .5f * std::cos(float(2 * pi) * p);
                const float shifted = pitch[c].read(float(sr) * (.012f + .03f * p)) * w
                                    + pitch[c].read(float(sr) * (.012f + .03f * q)) * (1 - w);
                result[c] = (x[c] + reverb * roomWet + delayed * echoWet + shifted * pitchWet) * out;
            }
            left[n] = result[0]; right[n] = result[1];
        }
    }
private:
    double sr = 44100;
    std::array<Biquad, 2> low, high;
    std::array<DelayLine, 2> echo, pitch;
    std::array<std::array<DelayLine, 4>, 2> room;
    std::array<std::array<float, 4>, 2> damping{};
    std::array<Smoother, 6> amount, enabled;
    Smoother input, output, delayTime;
    float env = 0, phase = 0;
    unsigned coefficientCounter = 0;
};
}
