cmake_minimum_required(VERSION 3.16)

project(FingeringOptimizer LANGUAGES CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenMP REQUIRED)

add_library(MidiParser STATIC
    midi_parser/src/Binasc.cpp
    midi_parser/src/MidiEvent.cpp
    midi_parser/src/MidiEventList.cpp
    midi_parser/src/MidiFile.cpp
    midi_parser/src/MidiMessage.cpp
)

target_include_directories(MidiParser
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/midi_parser/include
)

add_library(FingeringCore STATIC
    src/Engine.cpp
    src/Format.cpp
    src/NoteExtractor.cpp
    src/Optimizer.cpp
)

target_include_directories(FingeringCore
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(FingeringCore
    PUBLIC
        MidiParser
        OpenMP::OpenMP_CXX
)

add_executable(FingeringMain
    Main.cpp
)

target_link_libraries(FingeringMain
    PRIVATE
        FingeringCore
)

add_executable(Benchmark
    bench/Benchmark.cpp
)

target_link_libraries(Benchmark
    PRIVATE
        FingeringCore
)

find_package(Python3 COMPONENTS Interpreter Development QUIET)

if (Python3_FOUND)
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
        OUTPUT_VARIABLE pybind11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE pybind11_CMAKE_DIR_RESULT
    )

    if (pybind11_CMAKE_DIR_RESULT EQUAL 0)
        list(APPEND CMAKE_PREFIX_PATH ${pybind11_CMAKE_DIR})
        find_package(pybind11 CONFIG QUIET)
    endif()
endif()

if (pybind11_FOUND)
    pybind11_add_module(fingering_optimizer python/Bindings.cpp)

    target_link_libraries(fingering_optimizer
        PRIVATE
            FingeringCore
    )

    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_path('purelib'))"
        OUTPUT_VARIABLE Python3_SITE_PACKAGES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    add_custom_command(TARGET fingering_optimizer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fingering_optimizer> ${Python3_SITE_PACKAGES}/
    )
else()
    message(STATUS "pybind11 not found - skipping Python module target")
endif()
