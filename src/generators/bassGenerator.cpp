#include "bassGenerator.hpp"

#include "../core/random.hpp"

#include <algorithm>
#include <vector>

namespace {

std::vector<int> createPattern(
    const Chord& chord,
    int pattern) {

    const int root =
        chord.root - 24;

    const int fifth =
        root + 7;

    const int octave =
        root + 12;

    switch (pattern) {

        case 0:
            return {root};

        case 1:
            return {root, fifth};

        case 2:
            return {
                root,
                octave,
                fifth,
                octave
            };

        case 3:
            return {
                root,
                fifth,
                root,
                fifth
            };

        case 4:
            return {
                root,
                fifth,
                octave,
                fifth
            };

        default:
            return {root};
    }
}

}

std::vector<Note> BassGenerator::generate(
    const std::vector<Chord>& progression) {

    std::vector<Note> bass;

    if (progression.empty())
        return bass;

    double time = 0.0;

    for (size_t i = 0;
         i < progression.size();
         ++i) {

        const Chord& chord =
            progression[i];

        int pattern =
            Random::integer(0, 4);

        auto notes =
            createPattern(chord, pattern);

        const double beatDuration =
            2.0 / notes.size();

        for (size_t n = 0;
             n < notes.size();
             ++n) {

            int pitch =
                notes[n];

            if (Random::integer(1, 100) <= 12)
                pitch += 12;

            double duration =
                beatDuration;

            if (Random::integer(1, 100) <= 20)
                duration *= 0.75;

            bass.push_back({
                pitch,
                time,
                duration,
                Random::integer(70, 95)
            });

            time += beatDuration;
        }
    }

    return bass;
}