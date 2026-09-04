# Fingering-Optimizer

A full-stack system for computing optimal classical guitar fingerings from a MIDI file, featuring a C++ engine at the core, exposed to Python via pybind11, served by a FastAPI + SQLAlchemy backend, and used through a Qt desktop client.

## Overview

Given a MIDI file, the engine parses each track into a sequence of time steps (notes/chords), then runs a dynammic programming (DP) algorithm over every valid string/fret assignment at each time step to find the globally optimal fingering path for a 6-string classical guitar in standard tuning. Optimality is measured by a 5-factor weighted cost model, taking into account string gauge, fret distance, hand movement between chords, neck position, and muted/skipped strings, and the per-time-step candidate search is parallelized with OpenMP.

## Architecture

- **`include/`, `src/`, `midi_parser`** - the core C++ engine, including MIDI parsing, the DP fingering optimizer, and ASCII tab formatting (with sustained note detection).
- **`python/`** - pybind11 bindings exposing the engine as an importable Python module (`fingering_optimizer`).
- **`api/`** - a FastAPI and SQLAlchemy backend providing JWT authentication, MIDI uploads, weight presets, optimization, and results history, backed by SQLite.
- **`qt-client/`** - a Qt6 desktop client that talks to the backend over HTTP, featuring user-login, uploading, weight-sliders, preset loading/saving, and scrollable tablature results.
- **`bench/`** - a benchmark comparing the DP against a greedy baseline and single- vs. multi-threaded timing.
- **`Main.cpp`** - a CLI entry point that runs every MIDI file in `sample_midis/` through the engine and writes `.tab` files to `tab_outputs/`.

## Trying the App

**1. Start the backend and leave running:**

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
rm -rf build && mkdir build && cd build && cmake .. && make
cd ..
uvicorn api.main:app --reload
```

Interactive API docs are available at `http://127.0.0.1:8000/docs`; run the backend's test suite with `python -m pytest api/tests/`.

**2. In a separate terminal, build and launch the Qt client:**

```bash
cd qt-client
mkdir -p build && cd build
cmake ..
make
./FingeringOptimizerClient
```

Sign up, upload a MIDI file, adjust the weight sliders, and optimize.

## Exploring the C++ Engine Directly

This is not needed to use the app, but is useful for analyzing results without involving the rest of the stack.

```bash
mkdir -p build && cd build
cmake ..
make
./FingeringMain # processes every file in sample_midis/, writing the results to tab_outputs/
./Benchmark # DP vs. greedy cost comparison (+OpenMP)
```

## Acknowledgments / License

This project uses the Midifile library by Craig Stuart Sapp (https://github.com/craigsapp/midifile), licensed under the 2-clause BSD License. All original copyright notices in the library are preserved.
