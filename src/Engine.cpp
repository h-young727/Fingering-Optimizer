#include "Engine.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Fingering.h"
#include "Format.h"
#include "MidiFile.h"
#include "NoteExtractor.h"
#include "Optimizer.h"
#include "Track.h"
#include "Weights.h"

std::vector<TrackResult> optimizeMidiFile(const std::string& midiPath, const Weights& weights)
{
    smf::MidiFile midi;

    if (!midi.read(midiPath))
    {
        throw std::runtime_error("Error loading MIDI file: " + midiPath);
    }

    midi.linkNotePairs();

    std::vector<Track> tracks;
    std::vector<int> trackIndices;

    for (int t = 0; t < midi.getTrackCount(); t++)
    {
        Track track = extractTrack(midi, t);

        if (track.empty())
        {
            continue;
        }

        tracks.push_back(std::move(track));
        trackIndices.push_back(t);
    }

    std::vector<TrackResult> results(tracks.size());

    // Each track's DP is fully independent of the others, so this is genuinely parallel work
    #pragma omp parallel for
    for (int i = 0; i < static_cast<int>(tracks.size()); i++)
    {
        std::vector<Fingering> fingerings = getFingering(tracks[i], weights);
        std::string tab = formatTab(tracks[i], fingerings);
        results[i] = {trackIndices[i], tab};
    }

    return results;
}
