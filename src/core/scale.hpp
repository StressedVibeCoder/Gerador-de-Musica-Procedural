#ifndef SCALE_HPP
#define SCALE_HPP

#include <vector>
#include "chord.hpp"

enum class ScaleType {
    Major,
    Minor,
    Dorian,
    Phrygian,
    Lydian,
    Mixolydian,
    Aeolian,
    Locrian
};

struct Scale {
    int root;
    ScaleType type;
    std::vector<int> intervals;

    std::vector<int> getNotes() const {
        std::vector<int> notes;
        for (int interval : intervals)
            notes.push_back(root + interval);
        return notes;
    }

    std::vector<Chord> getDiatonicChords() const {
        std::vector<Chord> chords;
        const std::vector<ChordType> qualities = {
            ChordType::Major, ChordType::Minor, ChordType::Minor,
            ChordType::Major, ChordType::Major, ChordType::Minor,
            ChordType::Diminished
        };

        auto notes = getNotes();
        for (size_t i = 0; i < notes.size(); ++i)
            chords.push_back(Chord::create(notes[i], qualities[i], static_cast<int>(i)+1));
        return chords;
    }
};

#endif
