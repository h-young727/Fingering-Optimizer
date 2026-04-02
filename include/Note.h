#include <string_view>
#include <string>

struct Note {
    int pitch;
    int startTick;
    int tickCount;

    // Extracted from MIDI pitch -> musical note conversion website
    static constexpr std::string_view noteNames[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
    static constexpr std::string_view octaves[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};

    // MIDI pitch 21 = A0 (lowest note), so 20 is subtracted for a 1-based index
    std::string getName() const {
        return std::string(noteNames[((pitch - 20) % 12) - 1]);
    }

    // MIDI pitch 24 = C1 (octaves center around C), so 12 is subtracted for a 0-based index
    std::string getOctave() const {
        return std::string(octaves[(pitch - 12) / 12]);
    }

    std::string getNameAndOctave() const {
        return getName() + getOctave();
    }
};