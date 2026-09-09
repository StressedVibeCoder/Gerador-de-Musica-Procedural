#ifndef BASS_GENERATOR_HPP
#define BASS_GENERATOR_HPP

#include <vector>
#include "../core/note.hpp"
#include "../core/chord.hpp"

class BassGenerator {
public:
    static std::vector<Note> generate(const std::vector<Chord>& progression);
};

#endif
