cmake_minimum_required(VERSION 3.16)

project(GeradorDeMusicaProcedural VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(libs/libremidi)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/core/note.hpp
    src/core/scale.hpp
    src/core/chord.hpp
    src/core/random.hpp
    src/core/musicSettings.hpp
    src/generators/melodyGenerator.cpp
    src/generators/melodyGenerator.hpp
    src/generators/chordProgression.cpp
    src/generators/chordProgression.hpp
    src/generators/bassGenerator.cpp
    src/generators/bassGenerator.hpp
    src/composition/composition.cpp
    src/composition/composition.hpp
    src/midi/midiExporter.cpp
    src/midi/midiExporter.hpp
    src/midi/midiPlayer.cpp
    src/midi/midiPlayer.hpp
    src/audio/player.cpp
    src/audio/player.hpp
    src/app/app.cpp
    src/app/app.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

target_link_libraries(${PROJECT_NAME} PRIVATE libremidi)

if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl m)
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/midi/generated")
