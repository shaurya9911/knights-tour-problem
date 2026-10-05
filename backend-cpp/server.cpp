// A small HTTP/1.1 server built directly on POSIX sockets -- no external web
// framework. It understands just enough of HTTP to read a request line,
// headers, and body, and to route three endpoints for the Knight's Tour API:
//
//   GET  /api/health   -> {"status":"ok"}
//   POST /api/solve    -> runs one algorithm, returns its path + stats
//   POST /api/compare  -> runs both algorithms, returns both results
//
// CORS headers are added by hand so the React frontend (running on a
// different port) is allowed to call this server directly from the browser.

// Sockets are POSIX on Linux/macOS but Winsock on Windows. This block picks
// the right headers/types so the exact same server logic below compiles
// natively on all three -- no WSL required on Windows.
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define CLOSE_SOCKET closesocket
    #define INVALID_SOCK INVALID_SOCKET
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using socket_t = int;
    #define CLOSE_SOCKET close
    #define INVALID_SOCK (-1)
#endif

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "algorithms.h"
#include "json.h"
#include "response_builder.h"

namespace {

constexpr int PORT = 5000;
constexpr int MIN_BOARD = 5;
constexpr int MAX_BOARD = 12;

struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

// Reads a full HTTP request (headers + body) off a connected socket.
bool readRequest(socket_t clientFd, HttpRequest &req) {
    std::string raw;
    char buf[4096];

    // Read until we've seen the end of the headers ("\r\n\r\n").
    size_t headerEnd = std::string::npos;
    while (headerEnd == std::string::npos) {
        int n = recv(clientFd, buf, sizeof(buf), 0);
        if (n <= 0) return false;
        raw.append(buf, n);
        headerEnd = raw.find("\r\n\r\n");
        if (raw.size() > 1 << 20) return false; // safety cap on header size
    }

    std::istringstream headerStream(raw.substr(0, headerEnd));
    std::string requestLine;
    std::getline(headerStream, requestLine);
    if (!requestLine.empty() && requestLine.back() == '\r') requestLine.pop_back();

    std::istringstream lineStream(requestLine);
    std::string httpVersion;
    lineStream >> req.method >> req.path >> httpVersion;

    size_t contentLength = 0;
    std::string headerLine;
    while (std::getline(headerStream, headerLine)) {
        if (!headerLine.empty() && headerLine.back() == '\r') headerLine.pop_back();
        auto colon = headerLine.find(':');
        if (colon == std::string::npos) continue;
        std::string key = headerLine.substr(0, colon);
        std::string value = headerLine.substr(colon + 1);
        for (auto &c : key) c = std::tolower(static_cast<unsigned char>(c));
        // trim leading whitespace from value
        size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos) value = value.substr(start);
        if (key == "content-length") contentLength = std::stoul(value);
    }

    std::string bodySoFar = raw.substr(headerEnd + 4);
    while (bodySoFar.size() < contentLength) {
        int n = recv(clientFd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        bodySoFar.append(buf, n);
    }
    req.body = bodySoFar.substr(0, contentLength);
    return true;
}

void sendResponse(socket_t clientFd, int statusCode, const std::string &statusText, const std::string &body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n"
        << "Content-Type: application/json\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        << "Access-Control-Allow-Headers: Content-Type\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << body;
    std::string response = out.str();
    send(clientFd, response.c_str(), static_cast<int>(response.size()), 0);
}

// Validates boardSize/startX/startY the same way the original Express API
// did. Returns an error message, or an empty string if the input is valid.
std::string validateInput(int boardSize, int startX, int startY) {
    if (boardSize < MIN_BOARD || boardSize > MAX_BOARD) {
        return "boardSize must be an integer between " + std::to_string(MIN_BOARD) +
               " and " + std::to_string(MAX_BOARD);
    }
    if (startX < 0 || startY < 0 || startX >= boardSize || startY >= boardSize) {
        return "start position must be inside the board";
    }
    return "";
}

void handleRequest(socket_t clientFd, const HttpRequest &req) {
    if (req.method == "OPTIONS") {
        sendResponse(clientFd, 204, "No Content", "");
        return;
    }

    if (req.method == "GET" && req.path == "/api/health") {
        sendResponse(clientFd, 200, "OK", "{\"status\":\"ok\"}");
        return;
    }

    if (req.method == "POST" && (req.path == "/api/solve" || req.path == "/api/compare")) {
        JsonValue body;
        try {
            body = parseJson(req.body);
        } catch (const std::exception &e) {
            sendResponse(clientFd, 400, "Bad Request", errorToJson("Malformed JSON body"));
            return;
        }

        int boardSize = static_cast<int>(body.get("boardSize").asNumber(-1));
        int startX = static_cast<int>(body.get("startX").asNumber(-1));
        int startY = static_cast<int>(body.get("startY").asNumber(-1));

        std::string error = validateInput(boardSize, startX, startY);
        if (!error.empty()) {
            sendResponse(clientFd, 400, "Bad Request", errorToJson(error));
            return;
        }

        if (req.path == "/api/compare") {
            TourResult w = solveWarnsdorff(boardSize, startX, startY);
            TourResult b = solveBacktracking(boardSize, startX, startY);
            sendResponse(clientFd, 200, "OK", compareResultToJson(w, b));
        } else {
            std::string algorithm = body.get("algorithm").asString("backtracking");
            TourResult r = (algorithm == "warnsdorff")
                                ? solveWarnsdorff(boardSize, startX, startY)
                                : solveBacktracking(boardSize, startX, startY);
            sendResponse(clientFd, 200, "OK", tourResultToJson(r));
        }
        return;
    }

    sendResponse(clientFd, 404, "Not Found", errorToJson("No such endpoint"));
}

void handleClient(socket_t clientFd) {
    HttpRequest req;
    if (readRequest(clientFd, req)) {
        try {
            handleRequest(clientFd, req);
        } catch (const std::exception &e) {
            sendResponse(clientFd, 500, "Internal Server Error", errorToJson(e.what()));
        }
    }
    CLOSE_SOCKET(clientFd);
}

} // namespace

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }
#endif

    socket_t serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCK) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0) {
        std::cerr << "Failed to bind to port " << PORT << " (already in use?)\n";
        return 1;
    }

    if (listen(serverFd, 32) < 0) {
        std::cerr << "Failed to listen\n";
        return 1;
    }

    std::cout << "Knight's Tour C++ API listening on http://localhost:" << PORT << "\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        socket_t clientFd = accept(serverFd, reinterpret_cast<sockaddr *>(&clientAddr), &clientLen);
        if (clientFd == INVALID_SOCK) continue;
        // Handled synchronously (one request at a time) rather than on a
        // background thread -- simpler, and avoids relying on std::thread,
        // which isn't reliably supported by every Windows toolchain. Fine
        // for a local single-browser demo; the browser only ever has one
        // request in flight against this API at a time anyway.
        handleClient(clientFd);
    }

#ifdef _WIN32
    WSACleanup();
#endif
}
