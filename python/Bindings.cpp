#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "Engine.h"
#include "Weights.h"

namespace py = pybind11;

PYBIND11_MODULE(fingering_optimizer, m)
{
    m.doc() = "Guitar fingering optimizer C++ engine";

    py::class_<Weights>(m, "Weights")
        .def(py::init<>())
        .def_readwrite("string_weight", &Weights::stringWeight)
        .def_readwrite("fret_stretch", &Weights::fretStretch)
        .def_readwrite("transition", &Weights::transition)
        .def_readwrite("fret_position", &Weights::fretPosition)
        .def_readwrite("string_skip", &Weights::stringSkip);

    py::class_<TrackResult>(m, "TrackResult")
        .def_readonly("track_index", &TrackResult::trackIndex)
        .def_readonly("tab", &TrackResult::tab);

    m.def("optimize_midi_file", &optimizeMidiFile,
          py::arg("midi_path"), py::arg("weights") = Weights{},
          "Run every non-empty track in a MIDI file through the optimizer");
}
