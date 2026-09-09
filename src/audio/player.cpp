#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "player.hpp"
#include "audioEvent.hpp"

#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <vector>

namespace {

constexpr double SAMPLE_RATE = 44100.0;
constexpr double PI = 3.14159265358979323846;

struct State {
    const std::vector<AudioEvent>* events;
    double bpm;
    double currentTime;
};

double midiToFrequency(int midi) {
    return 440.0 * std::pow(2.0, (midi - 69) / 12.0);
}

double envelope(double time, double duration) {
    constexpr double attack = 0.01;
    constexpr double release = 0.08;

    if (time < attack)
        return time / attack;

    if (duration - time < release)
        return std::max(0.0, (duration - time) / release);

    return 1.0;
}

double renderEvent(const AudioEvent& event,
                   double time,
                   double quarterSeconds) {

    const double start = event.startTime * quarterSeconds;
    const double duration = event.duration * quarterSeconds;

    if (time < start || time >= start + duration)
        return 0.0;

    const double localTime = time - start;
    const double frequency = midiToFrequency(event.pitch);
    const double env = envelope(localTime, duration);

    const double wave =
        std::sin(2.0 * PI * frequency * localTime);

    return wave *
           env *
           (event.velocity / 127.0) *
           event.volume;
}

std::vector<AudioEvent> buildEvents(const Composition& composition) {

    std::vector<AudioEvent> events;

    /*
     * Melody
     */
    for (const auto& note : composition.melody) {
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

    /*
     * Bass
     */
    for (const auto& note : composition.bass) {
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

    /*
     * Chords
     *
     * The MIDI exporter creates each chord at:
     *
     *     i * 2 beats
     *
     * with:
     *
     *     duration = 2 beats
     *     velocity = 62
     */
    for (size_t i = 0; i < composition.chords.size(); ++i) {

        const auto& chord = composition.chords[i];

        const double start = static_cast<double>(i) * 2.0;

        for (int pitch : chord.getNotes()) {

            events.push_back({
                start,
                2.0,
                pitch,
                62,
                0.12
            });
        }
    }

    /*
     * Keep events ordered by their start time.
     */
    std::sort(
        events.begin(),
        events.end(),
        [](const AudioEvent& a, const AudioEvent& b) {
            return a.startTime < b.startTime;
        }
    );

    return events;
}

double compositionDuration(
    const Composition& composition) {

    double beats = 0.0;

    for (const auto& event : composition.melody) {
        beats = std::max(
            beats,
            event.startTime + event.duration
        );
    }

    for (const auto& event : composition.bass) {
        beats = std::max(
            beats,
            event.startTime + event.duration
        );
    }

    beats = std::max(
        beats,
        composition.chords.size() * 2.0
    );

    const int bpm =
        std::clamp(
            composition.settings.bpm,
            20,
            300
        );

    return beats * 60.0 / bpm;
}

void callback(
    ma_device* device,
    void* output,
    const void* input,
    ma_uint32 frameCount) {

    auto* state =
        static_cast<State*>(device->pUserData);

    auto* samples =
        static_cast<float*>(output);

    (void)input;

    const double quarterSeconds =
        60.0 / state->bpm;

    for (ma_uint32 frame = 0;
         frame < frameCount;
         ++frame) {

        double sample = 0.0;

        /*
         * Render every event.
         *
         * This is still intentionally simple.
         * Later we can optimize this using an active-event
         * queue once the sound itself is correct.
         */
        for (const auto& event : *state->events) {

            const double start =
                event.startTime * quarterSeconds;

            /*
             * Since events are sorted, once we are past
             * the current time we can skip the rest.
             */
            if (start > state->currentTime)
                break;

            const double end =
                (event.startTime + event.duration)
                * quarterSeconds;

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

}

bool AudioPlayer::play(
    const Composition& composition) {

    const double duration =
        compositionDuration(composition);

    if (duration <= 0.0)
        return false;

    /*
     * Convert Composition -> AudioEvents before
     * starting the audio device.
     */
    auto events = buildEvents(composition);

    if (events.empty())
        return false;

    const int bpm =
        std::clamp(
            composition.settings.bpm,
            20,
            300
        );

    State state{
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

    config.playback.channels = 2;

    config.sampleRate =
        static_cast<ma_uint32>(SAMPLE_RATE);

    config.dataCallback =
        callback;

    config.pUserData =
        &state;

    ma_device device{};

    if (ma_device_init(
            nullptr,
            &config,
            &device
        ) != MA_SUCCESS) {

        return false;
    }

    if (ma_device_start(&device) != MA_SUCCESS) {

        ma_device_uninit(&device);

        return false;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(
            static_cast<int>(
                duration * 1000.0
            ) + 150
        )
    );

    ma_device_uninit(&device);

    return true;
}

