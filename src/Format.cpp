#include "Format.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "Fingering.h"
#include "Note.h"
#include "Track.h"

static const Note* findNoteByPitch(const std::set<Note>& timeStep, int pitch)
{
    Note key;
    key.pitch = pitch;
    key.startTick = 0;
    key.tickCount = 0;

    auto it = timeStep.find(key);
    return (it != timeStep.end()) ? &(*it) : nullptr;
}

// Detects sustained notes for more descriptive tablature
static bool isTieContinuation(const Track& track, const std::vector<Fingering>& fingerings, int t, int s)
{
    if (t == 0)
    {
        return false;
    }

    if (!fingerings[t - 1].isStringUsed(s) || !fingerings[t].isStringUsed(s))
    {
        return false;
    }

    if (fingerings[t - 1].getFret(s) != fingerings[t].getFret(s))
    {
        return false;
    }

    int pitch = fingerings[t].getPitch(s);
    const Note* prevNote = findNoteByPitch(track[t - 1], pitch);
    const Note* currNote = findNoteByPitch(track[t], pitch);

    if (prevNote == nullptr || currNote == nullptr)
    {
        return false;
    }

    return prevNote->startTick + prevNote->tickCount == currNote->startTick;
}

std::string formatTab(const Track& track, const std::vector<Fingering>& fingerings)
{
    // Labels and indices both ordered high to low, matching standard tab notation
    const std::string labels[] = {"e", "B", "G", "D", "A", "E"};
    const int stringOrder[] = {5, 4, 3, 2, 1, 0};

    int totalTimeSteps = static_cast<int>(fingerings.size());
    int timeStepsPerLine = 16;

    std::string tab;

    for (int start = 0; start < totalTimeSteps; start += timeStepsPerLine)
    {
        int end = std::min(start + timeStepsPerLine, totalTimeSteps);

        for (int i = 0; i < 6; i++)
        {
            int s = stringOrder[i];
            std::string line = labels[i] + "|";

            for (int t = start; t < end; t++)
            {
                if (fingerings[t].isStringUsed(s))
                {
                    int fret = fingerings[t].getFret(s);
                    bool tied = isTieContinuation(track, fingerings, t, s);

                    if (fret < 10)
                    {
                        line += tied ? "~" : "-";
                        line += std::to_string(fret) + "--";
                    }
                    else
                    {
                        line += std::to_string(fret);
                        line += tied ? "~-" : "--";
                    }
                }
                else
                {
                    line += "----";
                }
            }

            line += "|\n";
            tab += line;
        }

        tab += "\n";
    }

    return tab;
}

void writeTab(const Track& track, const std::vector<Fingering>& fingerings, const std::string& filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: could not write to file " << filename << "\n";
        return;
    }

    file << formatTab(track, fingerings);
    file.close();
}
