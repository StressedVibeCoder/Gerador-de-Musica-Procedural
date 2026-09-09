#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "../composition/composition.hpp"

class AudioPlayer {
public:
    static bool play(const Composition& composition);
};

#endif
