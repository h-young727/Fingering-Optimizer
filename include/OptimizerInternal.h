#pragma once

#include <set>
#include <vector>

#include "Fingering.h"
#include "Note.h"
#include "Weights.h"

// Exposes Optimizer.cpp's internals exclusively for the benchmark
std::vector<Fingering> generateCandidates(const std::set<Note>& timeStep, int t);
double localCost(const Fingering& fingering, const Weights& weights);
double getCost(const Fingering& prev, const Fingering& current, const Weights& weights);
