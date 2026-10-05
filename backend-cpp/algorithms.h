#pragma once
#include <vector>
#include <string>
#include <utility>

// Result of running one Knight's Tour algorithm.
struct TourResult {
    std::string algorithm;
    bool success = false;
    bool timedOut = false;
    std::vector<std::pair<int, int>> path; // (x, y) in visit order
    int steps = 0;
    long long nodesExplored = -1; // -1 means "not applicable" (Warnsdorff doesn't report this)
    long long timeMs = 0;
};

// Exhaustive recursive backtracking. Candidate moves at each step are tried
// in Warnsdorff order (fewest onward options first) purely to keep the
// search tractable on boards up to 12x12 within a bounded time/node budget.
TourResult solveBacktracking(int n, int startX, int startY,
                              long long nodeLimit = 2000000,
                              long long timeLimitMs = 4000);

// Greedy Warnsdorff's heuristic: always move to the reachable square with
// the fewest onward moves. No backtracking -- O(n^2) overall.
TourResult solveWarnsdorff(int n, int startX, int startY);
