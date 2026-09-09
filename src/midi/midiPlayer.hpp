#ifndef MIDI_PLAYER_HPP
#define MIDI_PLAYER_HPP

#include <string>

class MidiPlayer {
public:
    static bool playFile(const std::string& filename);
};

#endif
