#include "MidiFile.h"
#include "NoteExtractor.h"
#include <iostream>
#include <string>

int main() {
    smf::MidiFile midi;

    if (!midi.read("../Bach_Cantate_BWV147.mid")) {
        std::cerr << "Error loading MIDI file." << "\n";
        return 1;
    }

    midi.linkNotePairs();

    // Extract all tracks
    std::vector<Track> tracks;
    for (int t = 0; t < midi.getTrackCount(); t++) {
        tracks.push_back(extractTrack(midi, t));
    }

    // Print number of time steps for each track
    for (int t = 0; t < tracks.size(); t++) {
        std::cout << "Track " << t << ": " << tracks[t].size() << " time steps\n";
    }

    return 0;
}