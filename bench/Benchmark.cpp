#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <string>
#include <vector>

#include "Fingering.h"
#include "MidiFile.h"
#include "NoteExtractor.h"
#include "Optimizer.h"
#include "OptimizerInternal.h"
#include "Track.h"
#include "Weights.h"

static std::vector<Fingering> getFingeringGreedy(const Track& track, const Weights& weights)
{
    std::vector<Fingering> result;
    result.reserve(track.size());

    Fingering prev;
    bool hasPrev = false;

    for (int t = 0; t < static_cast<int>(track.size()); t++)
    {
        std::vector<Fingering> candidates = generateCandidates(track[t], t);

        if (candidates.empty())
        {
            candidates.push_back(Fingering{});
        }

        Fingering best = candidates[0];
        double bestCost = hasPrev ? getCost(prev, candidates[0], weights) : localCost(candidates[0], weights);

        for (std::size_t i = 1; i < candidates.size(); i++)
        {
            double cost = hasPrev ? getCost(prev, candidates[i], weights) : localCost(candidates[i], weights);

            if (cost < bestCost)
            {
                bestCost = cost;
                best = candidates[i];
            }
        }

        result.push_back(best);
        prev = best;
        hasPrev = true;
    }

    return result;
}

static double scoreSequence(const std::vector<Fingering>& fingerings, const Weights& weights)
{
    if (fingerings.empty())
    {
        return 0.0;
    }

    double total = localCost(fingerings[0], weights);

    for (std::size_t t = 1; t < fingerings.size(); t++)
    {
        total += getCost(fingerings[t - 1], fingerings[t], weights);
    }

    return total;
}

struct NamedTrack
{
    std::string label;
    Track track;
};

static std::vector<NamedTrack> loadAllTracks(const std::filesystem::path& inputDir)
{
    std::vector<NamedTrack> tracks;

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(inputDir))
    {
        if (entry.path().extension() != ".mid")
        {
            continue;
        }

        smf::MidiFile midi;

        if (!midi.read(entry.path().string()))
        {
            std::cerr << "Error loading MIDI file: " << entry.path() << "\n";
            continue;
        }

        midi.linkNotePairs();

        for (int t = 0; t < midi.getTrackCount(); t++)
        {
            Track track = extractTrack(midi, t);

            if (track.empty())
            {
                continue;
            }

            std::string label = entry.path().filename().string() + " track " + std::to_string(t);
            tracks.push_back({label, track});
        }
    }

    return tracks;
}

struct TrackResult
{
    std::string label;
    int timeSteps;
    double greedyCost;
    double dpCost;
    double improvement;
};

static std::vector<TrackResult> runCostComparison(const std::vector<NamedTrack>& tracks, const Weights& weights)
{
    std::vector<TrackResult> results;

    for (const NamedTrack& namedTrack : tracks)
    {
        std::vector<Fingering> greedyResult = getFingeringGreedy(namedTrack.track, weights);
        std::vector<Fingering> dpResult = getFingering(namedTrack.track, weights);

        double greedyCost = scoreSequence(greedyResult, weights);
        double dpCost = scoreSequence(dpResult, weights);

        if (greedyCost <= 0.0)
        {
            continue;
        }

        double improvement = (greedyCost - dpCost) / greedyCost * 100.0;
        results.push_back({namedTrack.label, static_cast<int>(namedTrack.track.size()), greedyCost, dpCost, improvement});
    }

    return results;
}

struct ThreadResult
{
    int threads;
    double msPerIteration;
    double speedup;
};

static std::vector<ThreadResult> runTimingSweep(const std::vector<NamedTrack>& tracks, const Weights& weights)
{
    int maxThreads = omp_get_max_threads();
    std::vector<int> threadCounts = {1, 2, 4, 8, 16};

    threadCounts.erase(std::remove_if(threadCounts.begin(), threadCounts.end(),
                            [maxThreads](int t) { return t > maxThreads; }),
        threadCounts.end());

    if (threadCounts.empty() || threadCounts.back() != maxThreads)
    {
        threadCounts.push_back(maxThreads);
    }

    std::vector<ThreadResult> results;
    double baselineMs = 0.0;

    for (int threads : threadCounts)
    {
        omp_set_num_threads(threads);

        auto start = std::chrono::steady_clock::now();

        for (int iter = 0; iter < 20; iter++)
        {
            for (const NamedTrack& namedTrack : tracks)
            {
                getFingering(namedTrack.track, weights);
            }
        }

        auto end = std::chrono::steady_clock::now();
        double msPerIteration = std::chrono::duration<double, std::milli>(end - start).count() / 20.0;

        if (threads == 1)
        {
            baselineMs = msPerIteration;
        }

        results.push_back({threads, msPerIteration, baselineMs / msPerIteration});
    }

    omp_set_num_threads(maxThreads);

    return results;
}

static void printCostTable(const std::vector<TrackResult>& results)
{
    double totalGreedyCost = 0.0;
    double totalDpCost = 0.0;
    double averagePerTrack = 0.0;

    std::cout << "Fingering quality: DP vs. greedy\n"
               << std::string(78, '-') << "\n"
               << std::left << std::setw(38) << "track" << std::right << std::setw(8) << "steps" << std::setw(10)
               << "greedy" << std::setw(10) << "dp" << std::setw(12) << "improvement" << "\n"
               << std::string(78, '-') << "\n";

    for (const TrackResult& result : results)
    {
        std::cout << std::left << std::setw(38) << result.label << std::right << std::setw(8) << result.timeSteps
                   << std::setw(10) << result.greedyCost << std::setw(10) << result.dpCost << std::setw(11)
                   << result.improvement << "%\n";

        totalGreedyCost += result.greedyCost;
        totalDpCost += result.dpCost;
        averagePerTrack += result.improvement;
    }

    averagePerTrack /= static_cast<double>(results.size());
    double aggregateImprovement = (totalGreedyCost - totalDpCost) / totalGreedyCost * 100.0;

    std::cout << std::string(78, '-') << "\n"
               << "tracks compared: " << results.size() << "\n"
               << "average per-track improvement: " << averagePerTrack << "%\n"
               << "aggregate (cost-weighted) improvement: " << aggregateImprovement << "%\n";
}

static void printTimingTable(const std::vector<ThreadResult>& results)
{
    std::cout << "\nOpenMP thread scaling (20 iterations per row)\n"
               << std::string(34, '-') << "\n"
               << std::right << std::setw(8) << "threads" << std::setw(16) << "ms/iteration" << std::setw(10)
               << "speedup" << "\n"
               << std::string(34, '-') << "\n";

    for (const ThreadResult& result : results)
    {
        std::cout << std::setw(8) << result.threads << std::setw(16) << result.msPerIteration << std::setw(9)
                   << result.speedup << "x\n";
    }
}

int main()
{
    const std::filesystem::path inputDir = "../sample_midis";
    Weights weights;

    std::cout << std::fixed << std::setprecision(2);

    std::vector<NamedTrack> tracks = loadAllTracks(inputDir);

    std::cerr.rdbuf(nullptr);

    std::vector<TrackResult> trackResults = runCostComparison(tracks, weights);
    std::vector<ThreadResult> threadResults = runTimingSweep(tracks, weights);

    printCostTable(trackResults);
    printTimingTable(threadResults);

    return 0;
}
