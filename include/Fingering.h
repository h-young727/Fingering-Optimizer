#pragma once

// MIDI pitch of each open string, indexed E A D G B e (standard tuning)
inline constexpr int openStringPitch[6] = {40, 45, 50, 55, 59, 64};
inline constexpr int maxFret = 19;

struct Fingering
{
    // Fret per string (-1 = not played, 0 = open string, etc.)
    int strings[6] = {-1, -1, -1, -1, -1, -1};

    int getFret(int stringIdx) const
    {
        return strings[stringIdx];
    }

    bool isStringUsed(int stringIdx) const
    {
        return strings[stringIdx] > -1;
    }

    int getPitch(int stringIdx) const
    {
        return strings[stringIdx] + openStringPitch[stringIdx];
    }
};
