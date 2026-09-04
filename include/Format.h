#pragma once

#include <string>
#include <vector>

#include "Fingering.h"
#include "Track.h"

std::string formatTab(const Track& track, const std::vector<Fingering>& fingerings);
void writeTab(const Track& track, const std::vector<Fingering>& fingerings, const std::string& filename);
