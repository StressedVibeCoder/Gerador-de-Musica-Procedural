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
        {{2,4,5,6}, {10,25,35,30}}, // I
        {{4,5,6},    {35,40,25}},   // ii
        {{2,4,6},    {30,40,30}},    // iii
        {{1,2,5,6}, {30,15,40,15}},  // IV
        {{1,6},      {80,20}},       // V
        {{2,4,5},    {25,40,35}},    // vi
        {{1},        {100}}          // vii°
    };

    static const TransitionTable fallback{
        {1,2,4,5,6}, {20,20,20,20,20}
    };

    if (degree < 1 || degree > 7)
        return fallback;
    return transitions[degree];
}

int chooseTransition(const TransitionTable& t) {
    if (t.candidates.empty() || t.weights.empty())
        return 1;

    int total = 0;
    for (int w : t.weights) total += w;

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
    const Scale& scale, int length) {

    std::vector<Chord> progression;
    auto chords = scale.getDiatonicChords();

    if (chords.empty() || length <= 0)
        return progression;

    Chord current = chords[0];
    progression.push_back(current);

    for (int i = 1; i < length; ++i) {
        int selectedDegree = chooseTransition(getTransitions(current.degree));
        current = chords[selectedDegree - 1];
        progression.push_back(current);
    }

    return progression;
}

std::vector<Chord> ChordProgressionGenerator::generate(
    const Scale& scale, const MusicSettings& settings) {

    const double beatsPerSecond = settings.bpm / 60.0;
    const double totalBeats = settings.durationSeconds * beatsPerSecond;
    const int chordCount = std::max(1, static_cast<int>(totalBeats / 2.0));

    return generate(scale, chordCount);
}
