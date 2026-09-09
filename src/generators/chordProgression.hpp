#ifndef CHORD_PROGRESSION_HPP
#define CHORD_PROGRESSION_HPP

#include <vector>
#include "../core/chord.hpp"
#include "../core/scale.hpp"
#include "../core/musicSettings.hpp"

class ChordProgressionGenerator {
public:
    static std::vector<Chord> generate(const Scale& scale, int length);
    static std::vector<Chord> generate(const Scale& scale, const MusicSettings& settings);
};

#endif
