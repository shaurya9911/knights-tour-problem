#include "response_builder.h"
#include "json.h"
#include <sstream>

namespace {

std::string pathToJsonArray(const std::vector<std::pair<int, int>> &path) {
    std::ostringstream out;
    out << "[";
    for (size_t i = 0; i < path.size(); i++) {
        if (i) out << ",";
        out << "[" << path[i].first << "," << path[i].second << "]";
    }
    out << "]";
    return out.str();
}

std::string tourResultFields(const TourResult &r) {
    std::ostringstream out;
    out << "\"algorithm\":\"" << jsonEscape(r.algorithm) << "\","
        << "\"success\":" << (r.success ? "true" : "false") << ","
        << "\"timedOut\":" << (r.timedOut ? "true" : "false") << ","
        << "\"path\":" << pathToJsonArray(r.path) << ","
        << "\"steps\":" << r.steps << ",";
    if (r.nodesExplored >= 0) {
        out << "\"nodesExplored\":" << r.nodesExplored << ",";
    }
    out << "\"timeMs\":" << r.timeMs;
    return out.str();
}

} // namespace

std::string tourResultToJson(const TourResult &r) {
    std::ostringstream out;
    out << "{" << tourResultFields(r) << "}";
    return out.str();
}

std::string compareResultToJson(const TourResult &warnsdorff, const TourResult &backtracking) {
    std::ostringstream out;
    out << "{"
        << "\"warnsdorff\":{" << tourResultFields(warnsdorff) << "},"
        << "\"backtracking\":{" << tourResultFields(backtracking) << "}"
        << "}";
    return out.str();
}

std::string errorToJson(const std::string &message) {
    std::ostringstream out;
    out << "{\"error\":\"" << jsonEscape(message) << "\"}";
    return out.str();
}
