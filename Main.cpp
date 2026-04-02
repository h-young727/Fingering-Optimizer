#include "MidiFile.h"
#include "Note.h"

#include <iostream>
#include <iomanip>

// Currently just tests that a MIDI file can be loaded and useful information can be extracted
int main() {
    smf::MidiFile midi;

    if (!midi.read("../Bach_Cantate_BWV147.mid")) {
        std::cerr << "Error loading MIDI file." << "\n";
        return 1;
    }
    
    midi.linkNotePairs();

    std::cout << "File:        " << midi.getFilename() << "\n";
    std::cout << "Tracks per quarter:         " << midi.getTPQ() << "\n";
    std::cout << "Tracks:      " << midi.getTrackCount() << "\n";
    std::cout << "Duration:    " << std::fixed << std::setprecision(2) << midi.getFileDurationInSeconds() << " seconds\n";
    std::cout << "Duration:    " << midi.getFileDurationInQuarters() << " quarter notes\n\n";

    for (int t = 0; t < midi.getTrackCount(); t++) {
        int noteCount = 0;
        for (int i = 0; i < midi.getEventCount(t); i++) {
            smf::MidiEvent& ev = midi.getEvent(t, i);

            // Confirm that event is "note-on", i.e. an actual note is played
            if (ev.size() >= 3 && (ev[0] & 0xF0) == 0x90 && ev[2] > 0) {
                noteCount++;
            }
        }
        std::cout << "Track " << t << ": " << noteCount << " notes\n";
    }

    std::cout << "\n";

    // Test Note struct on track 1
    int noteCount = 0;
    for (int i = 0; i < midi.getEventCount(1) && noteCount < 10; i++) {
        smf::MidiEvent& ev = midi.getEvent(1, i);

        if (ev.size() >= 3 && (ev[0] & 0xF0) == 0x90 && ev[2] > 0) {
            Note note;
            note.pitch = ev[1];
            note.startTick = ev.tick;
            note.tickCount = ev.getLinkedEvent()->tick - ev.tick;
            std::cout << note.getName() << " + " << note.getOctave() << " = " << note.getNameAndOctave() << " (pitch: " << note.pitch << " | starting tick: " << note.startTick << " | tick count: " << note.tickCount << ")" << "\n";
            noteCount++;
        }
    }
    return 0;
}