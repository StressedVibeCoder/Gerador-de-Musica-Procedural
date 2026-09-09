#include "melodyGenerator.hpp"
#include <algorithm>
#include "../core/random.hpp"

std::vector<Note> MelodyGenerator::generate(
    const Scale& scale, int noteCount) {
    std::vector<Note> melody;
    auto scaleNotes = scale.getNotes();
    if (scaleNotes.empty() || noteCount <= 0)
        return melody;

    double time = 0.0;

    for (int i = 0; i < noteCount; ++i) {
        int pitch = Random::choice(scaleNotes);

        if (i > 0 && Random::integer(1, 100) <= 65) {
            const int previous = melody.back().pitch;
            int best = scaleNotes.front();
            int distance = 1000;

            for (int p : scaleNotes) {
                int d = std::abs(p - previous);

                if (d < distance) {
                    distance = d;
                    best = p;
                }
            }

            if (Random::integer(1, 100) <= 55)
                pitch = best;
        }

        double duration =
            (Random::integer(1, 100) <= 75)
                ? 0.5
                : 1.0;

        melody.push_back({
            pitch,
            time,
            duration,
            Random::integer(70, 110)
        });

        time += duration;
    }

    return melody;
}

std::vector<Note> MelodyGenerator::generate(
    const Scale& scale,
    const MusicSettings& settings) {

    const double beatsPerSecond =
        settings.bpm / 60.0;

    const double totalBeats =
        settings.durationSeconds * beatsPerSecond;

    const int noteCount =
        std::max(
            1,
            static_cast<int>(totalBeats / 0.5)
        );

    auto melody =
        generate(scale, noteCount);

    for (auto& note : melody) {

        if (note.startTime >= totalBeats) {
            note.duration = 0.0;
            continue;
        }

        note.duration =
            std::min(
                note.duration,
                totalBeats - note.startTime
            );
    }

    melody.erase(
        std::remove_if(
            melody.begin(),
            melody.end(),
            [](const Note& n) {
                return n.duration <= 0.0;
            }
        ),
        melody.end()
    );

    return melody;
}