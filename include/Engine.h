#pragma once

#include <string>
#include <vector>

#include "Weights.h"

struct TrackResult
{
    int trackIndex;
    std::string tab;
};

// Runs every non-empty track through the optimizer, throwing std::runtime_error if midiPath can't be read
std::vector<TrackResult> optimizeMidiFile(const std::string& midiPath, const Weights& weights = Weights{});
