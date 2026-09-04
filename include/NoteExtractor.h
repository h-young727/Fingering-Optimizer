#pragma once

#include "MidiFile.h"
#include "Track.h"

Track extractTrack(smf::MidiFile& midi, int trackIndex);