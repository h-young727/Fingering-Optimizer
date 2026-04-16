cmake_minimum_required(VERSION 3.10)
project(FingeringOptimizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${PROJECT_SOURCE_DIR}/midi_parser/include
    ${PROJECT_SOURCE_DIR}/include
)

add_library(MidiParser
    midi_parser/src/Binasc.cpp
    midi_parser/src/MidiEvent.cpp
    midi_parser/src/MidiEventList.cpp
    midi_parser/src/MidiFile.cpp
    midi_parser/src/MidiMessage.cpp
)

add_executable(FingeringMain 
    Main.cpp
    src/NoteExtractor.cpp
)

target_link_libraries(FingeringMain MidiParser)