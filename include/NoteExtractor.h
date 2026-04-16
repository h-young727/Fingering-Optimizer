#pragma once

#include "MidiFile.h"
#include "Track.h"
#include <string>

Track extractTrack(smf::MidiFile& midi, int trackIndex);