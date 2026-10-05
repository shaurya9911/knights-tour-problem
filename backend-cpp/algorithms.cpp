#include "algorithms.h"
#include <chrono>
#include <algorithm>
#include <tuple>

using Clock = std::chrono::steady_clock;

namespace {

// The 8 possible knight moves, as (dx, dy) offsets.
const int MOVES[8][2] = {
    {2, 1}, {1, 2}, {-1, 2}, {-2, 1},
    {-2, -1}, {-1, -2}, {1, -2}, {2, -1},
};

inline bool isInside(int x, int y, int n) {
    return x >= 0 && y >= 0 && x < n && y < n;
}

// Number of still-unvisited squares reachable from (x, y). Warnsdorff's rule
// uses this as the "accessibility" of a square.
int countOnwardMoves(const std::vector<std::vector<int>> &board, int x, int y, int n) {
    int count = 0;
    for (auto &m : MOVES) {
        int nx = x + m[0], ny = y + m[1];
        if (isInside(nx, ny, n) && board[ny][nx] == -1) count++;
    }
    return count;
}

} // namespace

// ---------------------------------------------------------------------
// Backtracking
// ---------------------------------------------------------------------
namespace {

struct BacktrackCtx {
    int n;
    std::vector<std::vector<int>> board;
    std::vector<std::pair<int, int>> path;
    long long nodesExplored = 0;
    long long nodeLimit;
    Clock::time_point startTime;
    long long timeLimitMs;
    bool timedOut = false;
};

bool backtrack(BacktrackCtx &ctx, int x, int y, int moveCount) {
    if (moveCount == ctx.n * ctx.n) return true;

    ctx.nodesExplored++;
    if (ctx.nodesExplored > ctx.nodeLimit ||
        std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - ctx.startTime).count() > ctx.timeLimitMs) {
        ctx.timedOut = true;
        return false;
    }

    // Order candidate moves by Warnsdorff heuristic (fewest onward moves first)
    // -- a standard optimization that keeps plain backtracking tractable.
    std::vector<std::tuple<int, int, int>> candidates; // (accessibility, nx, ny)
    for (auto &m : MOVES) {
        int nx = x + m[0], ny = y + m[1];
        if (isInside(nx, ny, ctx.n) && ctx.board[ny][nx] == -1) {
            candidates.emplace_back(countOnwardMoves(ctx.board, nx, ny, ctx.n), nx, ny);
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](auto &a, auto &b) { return std::get<0>(a) < std::get<0>(b); });

    for (auto &cand : candidates) {
        int nx = std::get<1>(cand), ny = std::get<2>(cand);
        ctx.board[ny][nx] = moveCount;
        ctx.path.push_back({nx, ny});

        if (backtrack(ctx, nx, ny, moveCount + 1)) return true;

        // undo -- this is the "backtrack" step
        ctx.board[ny][nx] = -1;
        ctx.path.pop_back();

        if (ctx.timedOut) return false;
    }
    return false;
}

} // namespace

TourResult solveBacktracking(int n, int startX, int startY, long long nodeLimit, long long timeLimitMs) {
    BacktrackCtx ctx;
    ctx.n = n;
    ctx.board.assign(n, std::vector<int>(n, -1));
    ctx.path.push_back({startX, startY});
    ctx.board[startY][startX] = 0;
    ctx.nodeLimit = nodeLimit;
    ctx.timeLimitMs = timeLimitMs;
    ctx.startTime = Clock::now();

    bool success = backtrack(ctx, startX, startY, 1);
    long long timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - ctx.startTime).count();

    TourResult result;
    result.algorithm = "backtracking";
    result.success = success;
    result.timedOut = ctx.timedOut;
    result.path = ctx.path;
    result.steps = static_cast<int>(ctx.path.size());
    result.nodesExplored = ctx.nodesExplored;
    result.timeMs = timeMs;
    return result;
}

// ---------------------------------------------------------------------
// Warnsdorff's heuristic
// ---------------------------------------------------------------------
TourResult solveWarnsdorff(int n, int startX, int startY) {
    auto startTime = Clock::now();
    std::vector<std::vector<int>> board(n, std::vector<int>(n, -1));

    int x = startX, y = startY;
    board[y][x] = 0;
    std::vector<std::pair<int, int>> path = {{x, y}};

    bool success = true;
    for (int move = 1; move < n * n; move++) {
        int bestX = -1, bestY = -1, bestAccess = 1 << 30;

        for (auto &m : MOVES) {
            int nx = x + m[0], ny = y + m[1];
            if (isInside(nx, ny, n) && board[ny][nx] == -1) {
                int acc = countOnwardMoves(board, nx, ny, n);
                if (acc < bestAccess) {
                    bestAccess = acc;
                    bestX = nx;
                    bestY = ny;
                }
            }
        }

        if (bestX == -1) { // stuck: no unvisited neighbour reachable
            success = false;
            break;
        }

        x = bestX;
        y = bestY;
        board[y][x] = move;
        path.push_back({x, y});
    }

    long long timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - startTime).count();

    TourResult result;
    result.algorithm = "warnsdorff";
    result.success = success;
    result.path = path;
    result.steps = static_cast<int>(path.size());
    result.timeMs = timeMs;
    return result;
}
