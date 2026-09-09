#include "miniaudio.h"

#include "midiPlayer.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>

namespace {

struct Event {
    double start = 0.0;
    double duration = 0.0;
    int pitch = 60;
    int velocity = 80;
};

uint16_t read16(const std::vector<uint8_t>& d, size_t& p) {
    if (p + 2 > d.size()) throw std::runtime_error("Invalid MIDI");
    uint16_t v = (d[p] << 8) | d[p+1];
    p += 2;
    return v;
}

uint32_t read32(const std::vector<uint8_t>& d, size_t& p) {
    if (p + 4 > d.size()) throw std::runtime_error("Invalid MIDI");
    uint32_t v = (uint32_t(d[p]) << 24) |
                 (uint32_t(d[p+1]) << 16) |
                 (uint32_t(d[p+2]) << 8) |
                 uint32_t(d[p+3]);
    p += 4;
    return v;
}

uint32_t readVLQ(const std::vector<uint8_t>& d, size_t& p) {
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        if (p >= d.size()) throw std::runtime_error("Invalid VLQ");
        uint8_t b = d[p++];
        value = (value << 7) | (b & 0x7F);
        if (!(b & 0x80)) return value;
    }
    throw std::runtime_error("Invalid VLQ");
}

std::vector<uint8_t> load(const std::string& filename) {
    std::ifstream f(filename, std::ios::binary);
    if (!f) return {};
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(f), {});
}

void parseTrack(const std::vector<uint8_t>& data,
                size_t begin, size_t end, uint16_t tpq,
                int& bpm, std::vector<Event>& events) {

    size_t p = begin;
    uint32_t tick = 0;
    uint8_t runningStatus = 0;
    struct Active { int pitch; int velocity; uint32_t start; };
    std::vector<Active> active;

    while (p < end) {
        tick += readVLQ(data, p);
        if (p >= end) break;

        uint8_t status = data[p++];
        if (status < 0x80) {
            if (!runningStatus)
                throw std::runtime_error("Invalid running status");
            --p;
            status = runningStatus;
        } else if (status < 0xF0) {
            runningStatus = status;
        }

        if (status == 0xFF) {
            if (p >= end) break;
            uint8_t type = data[p++];
            uint32_t len = readVLQ(data, p);
            if (p + len > end) throw std::runtime_error("Invalid meta event");

            if (type == 0x51 && len == 3) {
                uint32_t us = (uint32_t(data[p]) << 16) |
                              (uint32_t(data[p+1]) << 8) |
                              uint32_t(data[p+2]);
                if (us)
                    bpm = static_cast<int>(60000000.0 / us + 0.5);
            }
            p += len;
            continue;
        }

        if (status == 0xF0 || status == 0xF7) {
            uint32_t len = readVLQ(data, p);
            p += len;
            continue;
        }

        uint8_t command = status & 0xF0;
        if (command != 0x80 && command != 0x90 &&
            command != 0xA0 && command != 0xB0 &&
            command != 0xC0 && command != 0xD0 &&
            command != 0xE0)
            throw std::runtime_error("Unsupported MIDI event");

        if (p >= end) throw std::runtime_error("Invalid MIDI event");
        uint8_t a = data[p++];

        uint8_t b = 0;
        if (command != 0xC0 && command != 0xD0) {
            if (p >= end) throw std::runtime_error("Invalid MIDI event");
            b = data[p++];
        }

        if (command == 0x90 && b != 0) {
            active.push_back({a, b, tick});
        } else if (command == 0x80 || (command == 0x90 && b == 0)) {
            for (auto it = active.begin(); it != active.end(); ++it) {
                if (it->pitch == a) {
                    double start = double(it->start) / tpq;
                    double duration = double(tick - it->start) / tpq;
                    events.push_back({start, duration, a, it->velocity});
                    active.erase(it);
                    break;
                }
            }
        }
    }
}

constexpr double PI = 3.14159265358979323846;

struct State {
    const std::vector<Event>* events;
    double bpm;
    double time;
};

double frequency(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

void callback(ma_device* device, void* output,
              const void*, ma_uint32 frames) {
    auto* s = static_cast<State*>(device->pUserData);
    auto* out = static_cast<float*>(output);
    const double q = 60.0 / s->bpm;

    for (ma_uint32 i = 0; i < frames; ++i) {
        double sample = 0.0;

        for (const auto& e : *s->events) {
            double start = e.start * q;
            double dur = e.duration * q;
            if (s->time < start || s->time >= start + dur)
                continue;

            double local = s->time - start;
            double env = std::min(1.0, local / 0.01);
            if (dur - local < 0.08)
                env = std::min(env, std::max(0.0, (dur-local)/0.08));

            sample += std::sin(2.0 * PI * frequency(e.pitch) * local)
                      * env * (e.velocity / 127.0) * 0.12;
        }

        sample = std::clamp(sample, -0.85, 0.85);
        out[i*2] = static_cast<float>(sample);
        out[i*2+1] = static_cast<float>(sample);
        s->time += 1.0 / 44100.0;
    }
}

}

bool MidiPlayer::playFile(const std::string& filename) {
    try {
        auto data = load(filename);
        if (data.size() < 14) return false;

        size_t p = 0;
        if (std::string(data.begin(), data.begin()+4) != "MThd")
            return false;
        p = 4;

        uint32_t headerSize = read32(data, p);
        if (headerSize < 6 || p + headerSize > data.size())
            return false;

        uint16_t format = read16(data, p);
        uint16_t tracks = read16(data, p);
        uint16_t tpq = read16(data, p);

        if (format > 1 || tpq == 0) return false;
        p = 8 + headerSize;

        int bpm = 120;
        std::vector<Event> events;

        for (uint16_t i = 0; i < tracks && p + 8 <= data.size(); ++i) {
            if (std::string(data.begin()+p, data.begin()+p+4) != "MTrk")
                return false;
            p += 4;
            uint32_t len = read32(data, p);
            if (p + len > data.size()) return false;
            parseTrack(data, p, p + len, tpq, bpm, events);
            p += len;
        }

        if (events.empty()) return false;

        std::sort(events.begin(), events.end(),
                  [](const Event& a, const Event& b) {
                      return a.start < b.start;
                  });

        double end = 0.0;
        for (const auto& e : events)
            end = std::max(end, e.start + e.duration);

        State state{&events, static_cast<double>(std::clamp(bpm, 20, 300)), 0.0};

        ma_device_config config =
            ma_device_config_init(ma_device_type_playback);
        config.playback.format = ma_format_f32;
        config.playback.channels = 2;
        config.sampleRate = 44100;
        config.dataCallback = callback;
        config.pUserData = &state;

        ma_device device{};
        if (ma_device_init(nullptr, &config, &device) != MA_SUCCESS)
            return false;

        if (ma_device_start(&device) != MA_SUCCESS) {
            ma_device_uninit(&device);
            return false;
        }

        double seconds = end * 60.0 / state.bpm;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<int>(seconds*1000)+150));

        ma_device_uninit(&device);
        return true;

    } catch (...) {
        return false;
    }
}
