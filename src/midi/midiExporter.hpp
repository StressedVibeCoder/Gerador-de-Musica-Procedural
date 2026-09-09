#ifndef MIDI_EXPORTER_HPP
#define MIDI_EXPORTER_HPP

#include <string>
#include "../composition/composition.hpp"

class MidiExporter {
public:
    static bool exportToFile(const Composition& composition,
                             const std::string& filename);

    static std::string exportNext(const Composition& composition,
                                   const std::string& directory);
};

#endif
