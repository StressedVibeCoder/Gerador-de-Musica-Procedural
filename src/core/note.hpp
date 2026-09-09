#ifndef NOTE_HPP
#define NOTE_HPP

struct Note {
    int pitch = 60;
    double startTime = 0.0; // quarter notes
    double duration = 0.5;  // quarter notes
    int velocity = 90;
};

#endif
