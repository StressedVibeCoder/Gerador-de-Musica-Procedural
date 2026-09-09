#ifndef APP_HPP
#define APP_HPP

#include <filesystem>
#include <string>
#include "../composition/composition.hpp"

class App {
public:
    void run();

private:
    std::filesystem::path library = "midi/generated";

    void clearScreen() const;
    void pause() const;
    void mainMenu();
    void generateMenu();
    void libraryMenu();
    void settingsMenu();

    Composition generateComposition() const;
    void printComposition(const Composition& composition) const;

    int askInt(const std::string& prompt, int min, int max) const;
    double askDouble(const std::string& prompt, double min, double max) const;
};

#endif
