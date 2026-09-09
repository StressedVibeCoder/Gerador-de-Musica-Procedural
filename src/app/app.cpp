#include "app.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

#include "../audio/player.hpp"
#include "../core/scale.hpp"
#include "../generators/bassGenerator.hpp"
#include "../generators/chordProgression.hpp"
#include "../generators/melodyGenerator.hpp"
#include "../midi/midiExporter.hpp"
#include "../midi/midiPlayer.hpp"

namespace {

const char* reset = "\033[0m";
const char* cyan = "\033[36m";
const char* green = "\033[32m";
const char* yellow = "\033[33m";
const char* red = "\033[31m";

std::string rootName(int midi) {
    static const char* names[] =
        {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return names[(midi % 12 + 12) % 12];
}

Scale makeScale(int root) {
    return {
        root,
        ScaleType::Major,
        {0,2,4,5,7,9,11}
    };
}

}

void App::clearScreen() const {
    std::cout << "\033[2J\033[H";
}

void App::pause() const {
    std::cout << "\nPrima ENTER para continuar...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int App::askInt(const std::string& prompt, int min, int max) const {
    while (true) {
        std::cout << prompt;
        int value;
        if (std::cin >> value && value >= min && value <= max)
            return value;

        std::cout << red << "Valor inválido.\n" << reset;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

double App::askDouble(const std::string& prompt, double min, double max) const {
    while (true) {
        std::cout << prompt;
        double value;
        if (std::cin >> value && value >= min && value <= max)
            return value;

        std::cout << red << "Valor inválido.\n" << reset;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

Composition App::generateComposition() const {
    MusicSettings settings;

    std::cout << "\n" << cyan << "=== CONFIGURAÇÃO ===" << reset << "\n";

    std::cout << "Duração:\n"
              << "1. 30 segundos\n"
              << "2. 1 minuto\n"
              << "3. 2 minutos\n"
              << "4. 5 minutos\n"
              << "5. Personalizada\n";

    int d = askInt("Escolha: ", 1, 5);
    switch (d) {
        case 1: settings.durationSeconds = 30; break;
        case 2: settings.durationSeconds = 60; break;
        case 3: settings.durationSeconds = 120; break;
        case 4: settings.durationSeconds = 300; break;
        case 5: settings.durationSeconds =
            askDouble("Segundos (1-1800): ", 1, 1800); break;
    }

    std::cout << "\nBPM:\n"
              << "1. 80\n2. 100\n3. 120\n4. 140\n5. Personalizado\n";

    int b = askInt("Escolha: ", 1, 5);
    switch (b) {
        case 1: settings.bpm = 80; break;
        case 2: settings.bpm = 100; break;
        case 3: settings.bpm = 120; break;
        case 4: settings.bpm = 140; break;
        case 5: settings.bpm = askInt("BPM (40-240): ", 40, 240); break;
    }

    std::cout << "\nTonalidade:\n"
              << "1. C\n2. C#\n3. D\n4. D#\n5. E\n6. F\n"
              << "7. F#\n8. G\n9. G#\n10. A\n11. A#\n12. B\n";

    int key = askInt("Escolha: ", 1, 12);
    settings.root = 60 + (key - 1);

    Scale scale = makeScale(settings.root);

    auto chords = ChordProgressionGenerator::generate(scale, settings);
    auto melody = MelodyGenerator::generate(scale, settings);
    auto bass = BassGenerator::generate(chords);

    Composition composition;
    composition.settings = settings;
    composition.chords = std::move(chords);
    composition.melody = std::move(melody);
    composition.bass = std::move(bass);

    return composition;
}

void App::printComposition(const Composition& c) const {
    std::cout << "\n" << cyan << "=== MÚSICA ===" << reset << "\n";
    std::cout << "Tonalidade: " << rootName(c.settings.root) << " Major\n";
    std::cout << "BPM: " << c.settings.bpm << "\n";
    std::cout << "Duração: " << std::fixed << std::setprecision(1)
              << c.settings.durationSeconds << " s\n";
    std::cout << "Acordes: " << c.chords.size() << "\n";
    std::cout << "Baixo: " << c.bass.size() << " notas\n";
    std::cout << "Melodia: " << c.melody.size() << " notas\n";

    std::cout << "\nProgressão: ";
    for (const auto& chord : c.chords) {
        static const char* roman[] =
            {"", "I", "ii", "iii", "IV", "V", "vi", "vii°"};
        if (chord.degree >= 1 && chord.degree <= 7)
            std::cout << roman[chord.degree] << ' ';
    }
    std::cout << "\n";
}

void App::generateMenu() {
    clearScreen();
    Composition composition = generateComposition();

    clearScreen();
    printComposition(composition);

    std::cout << "\n"
              << "1. Reproduzir\n"
              << "2. Guardar MIDI\n"
              << "3. Reproduzir e guardar\n"
              << "0. Voltar\n";

    int choice = askInt("Escolha: ", 0, 3);

    if (choice == 1 || choice == 3) {
        std::cout << yellow << "\nA reproduzir...\n" << reset;
        if (!AudioPlayer::play(composition))
            std::cout << red << "Não foi possível iniciar o áudio.\n" << reset;
    }

    if (choice == 2 || choice == 3) {
        std::string file = MidiExporter::exportNext(composition, library.string());
        if (file.empty())
            std::cout << red << "Falha ao guardar MIDI.\n" << reset;
        else
            std::cout << green << "MIDI guardado: " << file << reset << "\n";
    }

    pause();
}

void App::libraryMenu() {
    while (true) {
        clearScreen();
        std::cout << cyan << "=== BIBLIOTECA MIDI ===" << reset << "\n";
        std::cout << "Pasta: " << library << "\n\n";

        std::vector<std::filesystem::path> files;

        if (std::filesystem::exists(library)) {
            for (const auto& entry : std::filesystem::directory_iterator(library)) {
                if (entry.is_regular_file()) {
                    auto ext = entry.path().extension().string();
                    if (ext == ".mid" || ext == ".midi")
                        files.push_back(entry.path());
                }
            }
        }

        std::sort(files.begin(), files.end());

        if (files.empty()) {
            std::cout << yellow << "Nenhum MIDI encontrado.\n" << reset;
            pause();
            return;
        }

        for (size_t i = 0; i < files.size(); ++i)
            std::cout << (i + 1) << ". " << files[i].filename().string() << "\n";

        std::cout << "0. Voltar\n";
        int choice = askInt("Escolha: ", 0, static_cast<int>(files.size()));

        if (choice == 0) return;

        clearScreen();
        std::cout << "A reproduzir: "
                  << files[choice - 1].filename().string() << "\n";

        if (!MidiPlayer::playFile(files[choice - 1].string()))
            std::cout << red << "Falha ao reproduzir o MIDI.\n" << reset;
        else
            std::cout << green << "Reprodução terminada.\n" << reset;

        pause();
    }
}

void App::settingsMenu() {
    while (true) {
        clearScreen();
        std::cout << cyan << "=== DEFINIÇÕES ===" << reset << "\n\n";
        std::cout << "Pasta da biblioteca: " << library << "\n\n";
        std::cout << "1. Alterar pasta\n";
        std::cout << "0. Voltar\n";

        int choice = askInt("Escolha: ", 0, 1);
        if (choice == 0) return;

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Nova pasta: ";
        std::string path;
        std::getline(std::cin, path);

        if (!path.empty()) {
            library = path;
            std::filesystem::create_directories(library);
        }
    }
}

void App::mainMenu() {
    while (true) {
        clearScreen();

        std::cout << cyan
                  << "╔══════════════════════════════════════╗\n"
                  << "║     GERADOR DE MÚSICA PROCEDURAL     ║\n"
                  << "╠══════════════════════════════════════╣\n"
                  << "║  1. Gerar música                     ║\n"
                  << "║  2. Reproduzir MIDI                  ║\n"
                  << "║  3. Biblioteca                       ║\n"
                  << "║  4. Definições                       ║\n"
                  << "║  0. Sair                             ║\n"
                  << "╚══════════════════════════════════════╝\n"
                  << reset;

        int choice = askInt("Escolha: ", 0, 4);

        switch (choice) {
            case 1: generateMenu(); break;
            case 2: libraryMenu(); break;
            case 3: libraryMenu(); break;
            case 4: settingsMenu(); break;
            case 0: return;
        }
    }
}

void App::run() {
    std::filesystem::create_directories(library);
    mainMenu();
}
