#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "player.hpp"
#include "audioEvent.hpp"

#include "../core/random.hpp"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <vector>

namespace {

constexpr double SAMPLE_RATE = 44100.0;
constexpr double PI = 3.14159265358979323846;

double midiToFrequency(int midi)
{
    return 440.0 * std::pow(
        2.0,
        (midi - 69) / 12.0
    );
}


double envelope(double time, double duration)
{
    constexpr double attack = 0.01;
    constexpr double release = 0.08;

    if (time < attack)
        return time / attack;

    if (duration - time < release)
        return std::max(
            0.0,
            (duration - time) / release
        );

    return 1.0;
}

double renderEvent(
    const AudioEvent& event,
    double currentTime,
    double quarterSeconds)
{
    const double start =
        event.startTime * quarterSeconds;

    const double duration =
        event.duration * quarterSeconds;

    const double end =
        start + duration;

    if (currentTime < start || currentTime >= end)
        return 0.0;

    const double localTime =
        currentTime - start;

    const double frequency =
        midiToFrequency(event.pitch);

    const double env =
        envelope(
            localTime,
            duration
        );

    const double wave =
        std::sin(
            2.0 *
            PI *
            frequency *
            localTime
        );

    return wave *
           env *
           (event.velocity / 127.0) *
           event.volume;
}

struct PlayerState
{
    const std::vector<AudioEvent>* events;
    double bpm;
    double currentTime;
};

enum class ChordPattern
{
    Sustained,
    TwoHits,
    ThreeHits,
    Syncopated,
    ShortHits,
    Broken
};


ChordPattern chooseChordPattern()
{
    const int value =
        Random::integer(1, 100);

    if (value <= 25)
        return ChordPattern::Sustained;

    if (value <= 45)
        return ChordPattern::TwoHits;

    if (value <= 62)
        return ChordPattern::ThreeHits;

    if (value <= 77)
        return ChordPattern::Syncopated;

    if (value <= 90)
        return ChordPattern::ShortHits;

    return ChordPattern::Broken;
}


void addChordHit(
    std::vector<AudioEvent>& events,
    const Chord& chord,
    double start,
    double duration,
    int velocity,
    double volume)
{
    for (int pitch : chord.getNotes())
    {
        events.push_back({
            start,
            duration,
            pitch,
            velocity,
            volume
        });
    }
}


void addChordPattern(
    std::vector<AudioEvent>& events,
    const Chord& chord,
    double start,
    ChordPattern pattern)
{
    constexpr double volume = 0.075;

    switch (pattern)
    {
        case ChordPattern::Sustained:
        {
            addChordHit(
                events,
                chord,
                start,
                2.0,
                Random::integer(48, 65),
                volume
            );

            break;
        }

        case ChordPattern::TwoHits:
        {
            addChordHit(
                events,
                chord,
                start,
                0.75,
                Random::integer(48, 65),
                volume
            );

            addChordHit(
                events,
                chord,
                start + 1.0,
                0.75,
                Random::integer(45, 62),
                volume
            );

            break;
        }

        case ChordPattern::ThreeHits:
        {
            addChordHit(
                events,
                chord,
                start,
                0.5,
                Random::integer(45, 62),
                volume
            );

            addChordHit(
                events,
                chord,
                start + 0.75,
                0.5,
                Random::integer(48, 65),
                volume
            );

            addChordHit(
                events,
                chord,
                start + 1.5,
                0.5,
                Random::integer(45, 62),
                volume
            );

            break;
        }

        case ChordPattern::Syncopated:
        {
            addChordHit(
                events,
                chord,
                start + 0.25,
                0.75,
                Random::integer(48, 65),
                volume
            );

            addChordHit(
                events,
                chord,
                start + 1.25,
                0.5,
                Random::integer(45, 62),
                volume
            );

            addChordHit(
                events,
                chord,
                start + 1.75,
                0.25,
                Random::integer(45, 60),
                volume
            );

            break;
        }

        case ChordPattern::ShortHits:
        {
            for (int i = 0; i < 4; ++i)
            {
                addChordHit(
                    events,
                    chord,
                    start + i * 0.5,
                    0.25,
                    Random::integer(42, 58),
                    volume
                );
            }

            break;
        }

        case ChordPattern::Broken:
        {
            const auto notes =
                chord.getNotes();

            for (size_t i = 0; i < notes.size(); ++i)
            {
                AudioEvent event{
                    start + i * 0.35,
                    0.65,
                    notes[i],
                    Random::integer(42, 58),
                    volume
                };

                events.push_back(event);
            }

            break;
        }
    }
}

std::vector<AudioEvent> buildEvents(
    const Composition& composition)
{
    std::vector<AudioEvent> events;

    for (const auto& note : composition.melody)
    {
        if (note.duration <= 0.0)
            continue;

        events.push_back({
            note.startTime,
            note.duration,
            note.pitch,
            note.velocity,
            0.12
        });
    }

    for (const auto& note : composition.bass)
    {
        if (note.duration <= 0.0)
            continue;

        events.push_back({
            note.startTime,
            note.duration,
            note.pitch,
            note.velocity,
            0.12
        });
    }

    for (size_t i = 0;
         i < composition.chords.size();
         ++i)
    {
        const Chord& chord =
            composition.chords[i];

        const double start =
            static_cast<double>(i) * 2.0;

        const ChordPattern pattern =
            chooseChordPattern();

        addChordPattern(
            events,
            chord,
            start,
            pattern
        );
    }

    std::sort(
        events.begin(),
        events.end(),
        [](const AudioEvent& a,
           const AudioEvent& b)
        {
            if (a.startTime != b.startTime)
                return a.startTime < b.startTime;

            return a.pitch < b.pitch;
        }
    );

    return events;
}

double getCompositionDuration(
    const Composition& composition)
{
    double beats = 0.0;

    for (const auto& note : composition.melody)
    {
        beats = std::max(
            beats,
            note.startTime + note.duration
        );
    }

    for (const auto& note : composition.bass)
    {
        beats = std::max(
            beats,
            note.startTime + note.duration
        );
    }

    if (!composition.chords.empty())
    {
        beats = std::max(
            beats,
            static_cast<double>(
                composition.chords.size()
            ) * 2.0
        );
    }

    if (beats <= 0.0)
        return 0.0;

    const int bpm =
        std::clamp(
            composition.settings.bpm,
            20,
            300
        );

    return beats *
           60.0 /
           static_cast<double>(bpm);
}

void audioCallback(
    ma_device* device,
    void* output,
    const void* input,
    ma_uint32 frameCount)
{
    auto* state =
        static_cast<PlayerState*>(
            device->pUserData
        );

    auto* samples =
        static_cast<float*>(output);

    (void)input;

    const double quarterSeconds =
        60.0 / state->bpm;

    for (ma_uint32 frame = 0;
         frame < frameCount;
         ++frame)
    {
        double sample = 0.0;

        for (const auto& event : *state->events)
        {
            const double start =
                event.startTime * quarterSeconds;

            const double end =
                (event.startTime + event.duration) *
                quarterSeconds;

            if (state->currentTime < start)
                continue;

            if (state->currentTime >= end)
                continue;

            sample += renderEvent(
                event,
                state->currentTime,
                quarterSeconds
            );
        }

        sample = std::tanh(sample);

        samples[frame * 2] =
            static_cast<float>(sample);

        samples[frame * 2 + 1] =
            static_cast<float>(sample);

        state->currentTime +=
            1.0 / SAMPLE_RATE;
    }
}

} // namespace

bool AudioPlayer::play(
    const Composition& composition)
{
    const double duration =
        getCompositionDuration(composition);

    if (duration <= 0.0)
        return false;

    auto events =
        buildEvents(composition);

    if (events.empty())
        return false;

    const int bpm =
        std::clamp(
            composition.settings.bpm,
            20,
            300
        );

    PlayerState state{
        &events,
        static_cast<double>(bpm),
        0.0
    };

    ma_device_config config =
        ma_device_config_init(
            ma_device_type_playback
        );

    config.playback.format =
        ma_format_f32;

    config.playback.channels =
        2;

    config.sampleRate =
        static_cast<ma_uint32>(
            SAMPLE_RATE
        );

    config.dataCallback =
        audioCallback;

    config.pUserData =
        &state;

    ma_device device{};

    if (ma_device_init(
            nullptr,
            &config,
            &device
        ) != MA_SUCCESS)
    {
        return false;
    }

    if (ma_device_start(
            &device
        ) != MA_SUCCESS)
    {
        ma_device_uninit(
            &device
        );

        return false;
    }

    const auto milliseconds =
        static_cast<int>(
            duration * 1000.0
        ) + 250;

    std::this_thread::sleep_for(
        std::chrono::milliseconds(
            milliseconds
        )
    );

    ma_device_uninit(
        &device
    );

    return true;
}
