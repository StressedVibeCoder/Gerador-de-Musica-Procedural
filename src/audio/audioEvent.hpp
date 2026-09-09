#ifndef AUDIO_EVENT_HPP
#define AUDIO_EVENT_HPP

struct AudioEvent {
    double startTime;
    double duration;
    int pitch;
    int velocity;
    double volume;
};

#endif