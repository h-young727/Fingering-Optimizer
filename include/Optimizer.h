#pragma once

#include <vector>

#include "Fingering.h"
#include "Track.h"
#include "Weights.h"

std::vector<Fingering> getFingering(const Track& track, const Weights& weights = Weights{});
