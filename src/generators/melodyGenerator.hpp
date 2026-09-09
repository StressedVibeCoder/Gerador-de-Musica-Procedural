#ifndef MELODY_GENERATOR_HPP
#define MELODY_GENERATOR_HPP

#include <vector>
#include "../core/note.hpp"
#include "../core/scale.hpp"
#include "../core/musicSettings.hpp"

class MelodyGenerator {
public:
    static std::vector<Note> generate(const Scale& scale, int noteCount);
    static std::vector<Note> generate(const Scale& scale, const MusicSettings& settings);
};

#endif
