#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Engine.h"

int main()
{
    const std::filesystem::path inputDir = "../sample_midis";
    const std::filesystem::path outputDir = "../tab_outputs";

    std::filesystem::create_directories(outputDir);

    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(inputDir))
    {
        if (entry.path().extension() != ".mid")
        {
            continue;
        }

        std::vector<TrackResult> results;

        try
        {
            results = optimizeMidiFile(entry.path().string());
        }
        catch (const std::runtime_error& e)
        {
            std::cerr << e.what() << "\n";
            continue;
        }

        for (const TrackResult& result : results)
        {
            std::string outputName = entry.path().stem().string() + "_track" + std::to_string(result.trackIndex) + ".tab";
            std::filesystem::path outputPath = outputDir / outputName;

            std::ofstream file(outputPath);

            if (!file.is_open())
            {
                std::cerr << "Error: could not write to file " << outputPath.string() << "\n";
                continue;
            }

            file << result.tab;
            file.close();

            std::cout << "Wrote " << outputPath.string() << "\n";
        }
    }

    return 0;
}
