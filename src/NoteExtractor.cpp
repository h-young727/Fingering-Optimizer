#include "NoteExtractor.h"
#include <iostream>

Track extractTrack(smf::MidiFile& midi, int trackIndex) {
    Track track;

    for (int i = 0; i < midi.getEventCount(trackIndex); i++) {
        smf::MidiEvent& ev = midi.getEvent(trackIndex, i);

        // Confirm that event is "note-on", i.e. an actual note is played
        if (ev.size() >= 3 && (ev[0] & 0xF0) == 0x90 && ev[2] > 0) {
            Note note;
            note.pitch = ev[1];
            note.startTick = ev.tick;
            note.tickCount = ev.getLinkedEvent()->tick - ev.tick;

            // Find existing time step (chord), else create a new one
            if (!track.empty() && track.back().begin()->startTick == ev.tick) {
                track.back().insert(note);
            } else {
                std::set<Note> timeStep;
                timeStep.insert(note);
                track.push_back(timeStep);
            }
        }
    }

    return track;
}