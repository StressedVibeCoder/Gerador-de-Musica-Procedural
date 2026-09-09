#include "chordProgression.hpp"

#include <algorithm>
#include <vector>

#include "../core/random.hpp"

namespace {

struct TransitionTable {
    std::vector<int> candidates;
    std::vector<int> weights;
};

const TransitionTable& getTransitions(int degree) {

    static const TransitionTable transitions[] = {

        {{}, {}},

        // I
        {{2, 4, 5, 6},
         {10, 30, 35, 25}},

        // ii
        {{1, 4, 5, 6},
         {10, 35, 35, 20}},

        // iii
        {{1, 2, 4, 6},
         {20, 25, 35, 20}},

        // IV
        {{1, 2, 5, 6},
         {30, 15, 40, 15}},

        // V
        {{1, 4, 6},
         {65, 20, 15}},

        // vi
        {{1, 2, 4, 5},
         {15, 25, 35, 25}},

        // vii°
        {{1, 5},
         {80, 20}}
    };

    static const TransitionTable fallback{
        {1, 2, 4, 5, 6},
        {20, 20, 20, 20, 20}
    };

    if (degree < 1 || degree > 7)
        return fallback;

    return transitions[degree];
}

int chooseTransition(const TransitionTable& t) {

    if (t.candidates.empty() || t.weights.empty())
        return 1;

    int total = 0;

    for (int weight : t.weights)
        total += weight;

    int value = Random::integer(1, total);

    int accumulated = 0;

    for (size_t i = 0; i < t.candidates.size(); ++i) {

        accumulated += t.weights[i];

        if (value <= accumulated)
            return t.candidates[i];
    }

    return t.candidates.back();
}

}

std::vector<Chord> ChordProgressionGenerator::generate(
    const Scale& scale,
    int length) {

    std::vector<Chord> progression;

    auto chords = scale.getDiatonicChords();

    if (chords.empty() || length <= 0)
        return progression;

    Chord current = chords[0];

    progression.push_back(current);

    int previousDegree = current.degree;

    for (int i = 1; i < length; ++i) {

        int selectedDegree;

        do {
            selectedDegree =
                chooseTransition(
                    getTransitions(current.degree)
                );

        } while (
            selectedDegree == previousDegree &&
            Random::integer(1, 100) <= 80
        );

        current = chords[selectedDegree - 1];

        progression.push_back(current);

        previousDegree = current.degree;
    }

    return progression;
}

std::vector<Chord> ChordProgressionGenerator::generate(
    const Scale& scale,
    const MusicSettings& settings) {

    const double beatsPerSecond =
        settings.bpm / 60.0;

    const double totalBeats =
        settings.durationSeconds * beatsPerSecond;

        const int chordCount = std::max(1, static_cast<int>(totalBeats / 4.0));

    return generate(
        scale,
        chordCount
    );
}