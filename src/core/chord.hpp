#ifndef CHORD_HPP
#define CHORD_HPP

#include <vector>

enum class ChordType {
    Major,
    Minor,
    Diminished,
    Augmented,
    Dominant7,
    Major7,
    Minor7
};

struct Chord {
    int root;
    ChordType type;
    std::vector<int> intervals;
    int degree;

    std::vector<int> getNotes() const {
        std::vector<int> notes;
        for (int interval : intervals)
            notes.push_back(root + interval);
        return notes;
    }

    static Chord create(int root, ChordType type, int degree = 0) {
        switch (type) {
            case ChordType::Major:      return {root, type, {0,4,7}, degree};
            case ChordType::Minor:      return {root, type, {0,3,7}, degree};
            case ChordType::Diminished: return {root, type, {0,3,6}, degree};
            case ChordType::Augmented:  return {root, type, {0,4,8}, degree};
            case ChordType::Dominant7:  return {root, type, {0,4,7,10}, degree};
            case ChordType::Major7:     return {root, type, {0,4,7,11}, degree};
            case ChordType::Minor7:     return {root, type, {0,3,7,10}, degree};
        }
        return {root, type, {}, degree};
    }
};

#endif
