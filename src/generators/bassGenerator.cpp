#include "bassGenerator.hpp"

std::vector<Note> BassGenerator::generate(
    const std::vector<Chord>& progression) {

    std::vector<Note> bass;
    double time = 0.0;

    for (const Chord& chord : progression) {
        bass.push_back({
            chord.root - 24,
            time,
            2.0,
            85
        });
        time += 2.0;
    }

    return bass;
}
