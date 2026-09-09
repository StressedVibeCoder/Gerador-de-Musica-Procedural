#ifndef COMPOSITION_HPP
#define COMPOSITION_HPP

#include <vector>
#include "../core/note.hpp"
#include "../core/chord.hpp"
#include "../core/musicSettings.hpp"

struct Composition {
    std::vector<Chord> chords;
    std::vector<Note> bass;
    std::vector<Note> melody;
    MusicSettings settings;
};

#endif
