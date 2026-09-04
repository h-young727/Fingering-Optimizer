#include "Optimizer.h"
#include "OptimizerInternal.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <omp.h>
#include <set>
#include <vector>

#include "Fingering.h"
#include "Note.h"
#include "Track.h"
#include "Weights.h"

static int median(std::vector<int> frets)
{
    if (frets.empty())
    {
        return 0;
    }

    std::sort(frets.begin(), frets.end());
    return frets[frets.size() / 2];
}

static double averageFretPosition(const std::vector<int>& usedFrets)
{
    if (usedFrets.empty())
    {
        return 0.0;
    }

    int sum = 0;

    for (int fret : usedFrets)
    {
        sum += fret;
    }

    return static_cast<double>(sum) / usedFrets.size();
}

static int countSkippedStrings(const Fingering& fingering)
{
    int minUsed = -1;
    int maxUsed = -1;

    for (int s = 0; s < 6; s++)
    {
        if (fingering.isStringUsed(s))
        {
            if (minUsed == -1)
            {
                minUsed = s;
            }

            maxUsed = s;
        }
    }

    if (minUsed == -1)
    {
        return 0;
    }

    int skipped = 0;

    for (int s = minUsed; s <= maxUsed; s++)
    {
        if (!fingering.isStringUsed(s))
        {
            skipped++;
        }
    }

    return skipped;
}

double localCost(const Fingering& fingering, const Weights& weights)
{
    int stringWeight = 0;
    std::vector<int> usedFrets;

    for (int s = 0; s < 6; s++)
    {
        if (fingering.isStringUsed(s))
        {
            stringWeight += (6 - s);

            if (fingering.getFret(s) > 0)
            {
                usedFrets.push_back(fingering.getFret(s));
            }
        }
    }

    int fretStretch = 0;

    if (usedFrets.size() > 1)
    {
        int minFret = *std::min_element(usedFrets.begin(), usedFrets.end());
        int maxFret = *std::max_element(usedFrets.begin(), usedFrets.end());
        fretStretch = maxFret - minFret;
    }

    return (stringWeight * weights.stringWeight) + (fretStretch * weights.fretStretch)
        + (averageFretPosition(usedFrets) * weights.fretPosition) + (countSkippedStrings(fingering) * weights.stringSkip);
}

static double transitionCost(const Fingering& prev, const Fingering& current, const Weights& weights)
{
    std::vector<int> prevFrets;
    std::vector<int> currentFrets;

    for (int s = 0; s < 6; s++)
    {
        if (prev.getFret(s) > 0)
        {
            prevFrets.push_back(prev.getFret(s));
        }

        if (current.getFret(s) > 0)
        {
            currentFrets.push_back(current.getFret(s));
        }
    }

    if (prevFrets.empty() || currentFrets.empty())
    {
        return 0;
    }

    int prevMedian = median(prevFrets);
    int currentMedian = median(currentFrets);

    int cost = 0;

    for (int s = 0; s < 6; s++)
    {
        bool usedInPrev = prev.getFret(s) > 0;
        bool usedInCurrent = current.getFret(s) > 0;

        if (usedInPrev && usedInCurrent)
        {
            cost += std::abs(prev.getFret(s) - current.getFret(s));
        }
        else if (usedInPrev && !usedInCurrent)
        {
            cost += std::abs(prev.getFret(s) - currentMedian);
        }
        else if (!usedInPrev && usedInCurrent)
        {
            cost += std::abs(current.getFret(s) - prevMedian);
        }
    }

    return cost * weights.transition;
}

double getCost(const Fingering& prev, const Fingering& current, const Weights& weights)
{
    return localCost(current, weights) + transitionCost(prev, current, weights);
}

static void getCombinations(int numStrings, int numNotes, int start, std::vector<int>& current, std::vector<std::vector<int>>& result)
{
    if (static_cast<int>(current.size()) == numNotes)
    {
        result.push_back(current);
        return;
    }

    for (int i = start; i < numStrings; i++)
    {
        current.push_back(i);
        getCombinations(numStrings, numNotes, i + 1, current, result);
        current.pop_back();
    }
}

static Fingering buildFingering(const std::set<Note>& timeStep, const std::vector<int>& stringAssignment)
{
    Fingering fingering;
    int i = 0;

    for (const Note& note : timeStep)
    {
        fingering.strings[stringAssignment[i]] = note.pitch - openStringPitch[stringAssignment[i]];
        i++;
    }

    return fingering;
}

struct Candidate
{
    Fingering fingering;
    double cost;
    int backPtr;
};

static std::set<Note> dropUnplayableNotes(const std::set<Note>& timeStep, int t)
{
    std::set<Note> playable;

    for (const Note& note : timeStep)
    {
        // A pitch outside every string's [open, open + maxFret] range can't be fretted on any string
        if (note.pitch < openStringPitch[0] || note.pitch > openStringPitch[5] + maxFret)
        {
            std::cerr << "Note pitch " << note.pitch << " outside guitar range, dropped from time step " << t << "\n";
            continue;
        }

        playable.insert(note);
    }

    return playable;
}

// Expands each string combination into every permutation to capture crossed voicings
static std::vector<std::vector<int>> getAssignments(const std::vector<std::vector<int>>& combinations)
{
    std::vector<std::vector<int>> assignments;

    for (const std::vector<int>& combination : combinations)
    {
        std::vector<int> permutation = combination;

        do
        {
            assignments.push_back(permutation);
        }
        while (std::next_permutation(permutation.begin(), permutation.end()));
    }

    return assignments;
}

std::vector<Fingering> generateCandidates(const std::set<Note>& timeStep, int t)
{
    std::set<Note> playableNotes = dropUnplayableNotes(timeStep, t);
    int numNotes = static_cast<int>(playableNotes.size());

    std::vector<std::vector<int>> combinations;
    std::vector<int> current;
    getCombinations(6, numNotes, 0, current, combinations);

    std::vector<std::vector<int>> assignments = getAssignments(combinations);

    std::vector<Fingering> candidates;
    candidates.reserve(assignments.size());

    for (const std::vector<int>& stringAssignment : assignments)
    {
        Fingering candidate = buildFingering(playableNotes, stringAssignment);

        bool valid = true;

        // isStringUsed() misreads an invalid fret as unused, so validate directly
        for (int stringIdx : stringAssignment)
        {
            int fret = candidate.getFret(stringIdx);

            if (fret < 0 || fret > maxFret)
            {
                valid = false;
                break;
            }
        }

        if (valid)
        {
            candidates.push_back(candidate);
        }
    }

    return candidates;
}

// Keeps every candidate per time step so the global optimum can be retrieved by backtracking
static std::vector<Fingering> dp(const Track& track, const Weights& weights)
{
    std::vector<Fingering> result(track.size());

    if (track.empty())
    {
        return result;
    }

    std::vector<std::vector<Candidate>> layers(track.size());

    for (int t = 0; t < static_cast<int>(track.size()); t++)
    {
        std::vector<Fingering> candidates = generateCandidates(track[t], t);

        if (candidates.empty())
        {
            std::cerr << "No valid fingering found for time step " << t << "\n";
            candidates.push_back(Fingering{});
        }

        std::vector<Candidate>& layer = layers[t];
        layer.resize(candidates.size());

        const std::vector<Candidate>* prevLayer = (t == 0) ? nullptr : &layers[t - 1];

        // Capped: fork/join overhead on this small, per-time-step region outweighs the benefit past ~4 threads
        #pragma omp parallel for num_threads(std::min(4, omp_get_max_threads()))
        for (int i = 0; i < static_cast<int>(candidates.size()); i++)
        {
            const Fingering& candidate = candidates[i];

            if (prevLayer == nullptr)
            {
                layer[i] = { candidate, localCost(candidate, weights), -1 };
                continue;
            }

            double bestCost = std::numeric_limits<double>::max();
            int bestPrev = 0;

            for (int p = 0; p < static_cast<int>(prevLayer->size()); p++)
            {
                double cost = (*prevLayer)[p].cost + getCost((*prevLayer)[p].fingering, candidate, weights);

                if (cost < bestCost)
                {
                    bestCost = cost;
                    bestPrev = p;
                }
            }

            layer[i] = { candidate, bestCost, bestPrev };
        }
    }

    int lastLayer = static_cast<int>(track.size()) - 1;
    int bestIdx = 0;
    double bestCost = std::numeric_limits<double>::max();

    for (int i = 0; i < static_cast<int>(layers[lastLayer].size()); i++)
    {
        if (layers[lastLayer][i].cost < bestCost)
        {
            bestCost = layers[lastLayer][i].cost;
            bestIdx = i;
        }
    }

    for (int t = lastLayer; t >= 0; t--)
    {
        result[t] = layers[t][bestIdx].fingering;
        bestIdx = layers[t][bestIdx].backPtr;
    }

    return result;
}

std::vector<Fingering> getFingering(const Track& track, const Weights& weights)
{
    return dp(track, weights);
}
