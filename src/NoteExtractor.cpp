#include "NoteExtractor.h"

#include <set>

#include "MidiEvent.h"
#include "MidiFile.h"
#include "Note.h"

Track extractTrack(smf::MidiFile& midi, int trackIndex)
{
    Track track;

    for (int i = 0; i < midi.getEventCount(trackIndex); i++)
    {
        smf::MidiEvent& ev = midi.getEvent(trackIndex, i);

        // Confirm that event is "note-on", i.e. an actual note is played
        if (ev.size() >= 3 && (ev[0] & 0xF0) == 0x90 && ev[2] > 0)
        {
            Note note;
            note.pitch = ev[1];

            // MIDI pitches below 21 (A0) fall outside standard 88-key range
            if (note.pitch < 21)
            {
                continue;
            }

            note.startTick = ev.tick;

            // Duration comes from the paired note-off event, not this note-on event alone
            note.tickCount = ev.getLinkedEvent()->tick - ev.tick;

            // Group into the same time step only if this note starts on the exact same tick
            if (!track.empty() && track.back().begin()->startTick == ev.tick)
            {
                track.back().insert(note);
            }
            else
            {
                std::set<Note> timeStep;
                timeStep.insert(note);
                track.push_back(timeStep);
            }
        }
    }

    return track;
}
