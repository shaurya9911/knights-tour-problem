#pragma once
#include <string>
#include "algorithms.h"

// Builds the same JSON shape the original Node/Express API returned, so the
// existing React frontend needs no changes to talk to this C++ backend.
std::string tourResultToJson(const TourResult &r);
std::string compareResultToJson(const TourResult &warnsdorff, const TourResult &backtracking);
std::string errorToJson(const std::string &message);
