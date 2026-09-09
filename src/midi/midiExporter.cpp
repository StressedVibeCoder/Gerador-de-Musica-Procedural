#include "midiExporter.hpp"

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

void write16(std::ofstream& f, uint16_t v) {
    f.put(static_cast<char>((v >> 8) & 0xFF));
    f.put(static_cast<char>(v & 0xFF));
}

void write32(std::ofstream& f, uint32_t v) {
    f.put(static_cast<char>((v >> 24) & 0xFF));
    f.put(static_cast<char>((v >> 16) & 0xFF));
    f.put(static_cast<char>((v >> 8) & 0xFF));
    f.put(static_cast<char>(v & 0xFF));
}

void vlq(std::vector<uint8_t>& out, uint32_t value) {
    uint8_t buffer[5];
    int n = 0;
    buffer[n++] = static_cast<uint8_t>(value & 0x7F);
    while ((value >>= 7) != 0)
        buffer[n++] = static_cast<uint8_t>((value & 0x7F) | 0x80);
    while (n--)
        out.push_back(buffer[n]);
}

void finish(std::vector<uint8_t>& track) {
    vlq(track, 0);
    track.insert(track.end(), {0xFF, 0x2F, 0x00});
}

void writeTrack(std::ofstream& f, const std::vector<uint8_t>& track) {
    f.write("MTrk", 4);
    write32(f, static_cast<uint32_t>(track.size()));
    for (uint8_t b : track) f.put(static_cast<char>(b));
}

std::vector<uint8_t> tempoTrack(int bpm) {
    std::vector<uint8_t> t;
    bpm = std::clamp(bpm, 20, 300);
    uint32_t us = static_cast<uint32_t>(60000000 / bpm);

    vlq(t, 0);
    t.insert(t.end(), {0xFF, 0x51, 0x03,
        static_cast<uint8_t>((us >> 16) & 0xFF),
        static_cast<uint8_t>((us >> 8) & 0xFF),
        static_cast<uint8_t>(us & 0xFF)});
    finish(t);
    return t;
}

std::vector<uint8_t> noteTrack(const std::vector<Note>& notes, uint8_t channel) {
    constexpr uint32_t TPQ = 480;
    std::vector<uint8_t> t;

    struct Event { uint32_t tick; bool on; int pitch; int velocity; };
    std::vector<Event> events;

    for (const auto& n : notes) {
        if (n.duration <= 0) continue;
        uint32_t start = static_cast<uint32_t>(std::llround(n.startTime * TPQ));
        uint32_t end = static_cast<uint32_t>(
            std::llround((n.startTime + n.duration) * TPQ));
        events.push_back({start, true, n.pitch, n.velocity});
        events.push_back({end, false, n.pitch, 0});
    }

    std::sort(events.begin(), events.end(),
        [](const Event& a, const Event& b) {
            if (a.tick != b.tick) return a.tick < b.tick;
            return !a.on && b.on;
        });

    uint32_t previous = 0;
    for (const auto& e : events) {
        vlq(t, e.tick - previous);
        t.push_back(static_cast<uint8_t>((e.on ? 0x90 : 0x80) | channel));
        t.push_back(static_cast<uint8_t>(std::clamp(e.pitch, 0, 127)));
        t.push_back(static_cast<uint8_t>(std::clamp(e.velocity, 0, 127)));
        previous = e.tick;
    }

    finish(t);
    return t;
}

std::vector<uint8_t> chordTrack(const Composition& c) {
    constexpr uint32_t TPQ = 480;
    std::vector<Note> notes;

    for (size_t i = 0; i < c.chords.size(); ++i) {
        const double start = static_cast<double>(i) * 2.0;
        for (int pitch : c.chords[i].getNotes())
            notes.push_back({pitch, start, 2.0, 62});
    }
    return noteTrack(notes, 0);
}

}

bool MidiExporter::exportToFile(
    const Composition& composition,
    const std::string& filename) {

    std::filesystem::path path(filename);
    if (path.has_parent_path())
        std::filesystem::create_directories(path.parent_path());

    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    constexpr uint16_t TPQ = 480;

    file.write("MThd", 4);
    write32(file, 6);
    write16(file, 1);
    write16(file, 4);
    write16(file, TPQ);

    writeTrack(file, tempoTrack(composition.settings.bpm));
    writeTrack(file, chordTrack(composition));
    writeTrack(file, noteTrack(composition.bass, 1));
    writeTrack(file, noteTrack(composition.melody, 2));

    return static_cast<bool>(file);
}

std::string MidiExporter::exportNext(
    const Composition& composition,
    const std::string& directory) {

    std::filesystem::create_directories(directory);

    for (int number = 1;; ++number) {
        auto filename = std::filesystem::path(directory) /
            ("musicaGerada" + std::to_string(number) + ".mid");

        if (!std::filesystem::exists(filename)) {
            if (exportToFile(composition, filename.string()))
                return filename.string();
            return {};
        }
    }
}
