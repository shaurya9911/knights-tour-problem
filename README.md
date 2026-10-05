# Knight's Tour — DSA Visualizer

A full-stack project (Node.js/Express backend + React/Vite frontend) that solves
and visualizes the **Knight's Tour problem**: moving a knight around an n×n
chessboard so it visits every square exactly once.

Two classic algorithms are implemented and compared:

| Algorithm | Idea | Complexity | Guaranteed? |
|---|---|---|---|
| **Backtracking** | Try a move, recurse, undo if stuck | Exponential worst case (bounded here with a node/time limit) | Yes, if a tour exists |
| **Warnsdorff's Heuristic** | Always move to the reachable square with the *fewest* onward moves | O(n²) | No (rare failure cases) |

## Project structure

```
knights-tour/
├── backend/                     # Node.js/Express version of the API
│   ├── server.js
│   ├── algorithms/
│   │   ├── backtracking.js      # recursive backtracking (Warnsdorff-ordered branching)
│   │   └── warnsdorff.js        # greedy Warnsdorff's rule
│   └── package.json
├── backend-cpp/                 # C++ version of the same API (see below)
│   ├── server.cpp                # raw-socket HTTP server, routing, CORS
│   ├── algorithms.h / .cpp       # backtracking + Warnsdorff, same logic as backend/
│   ├── json.h / .cpp             # small hand-written JSON parser
│   ├── response_builder.h / .cpp # builds JSON responses
│   └── Makefile
└── frontend/
    ├── src/
    │   ├── App.jsx               # controls, API calls, playback state
    │   ├── Board.jsx             # chessboard + animated knight + path overlay
    │   ├── App.css / index.css
    │   └── main.jsx
    ├── index.html
    ├── vite.config.js
    └── package.json
```

There are now **two interchangeable backends** — pick one, not both (they
both serve the same API on port 5000, so only run one at a time):

- **`backend/`** — Node.js + Express (JavaScript)
- **`backend-cpp/`** — the actual DSA logic written in **C++**, served over
  a small HTTP server built directly on POSIX sockets (no web framework, no
  external libraries — just the C++ standard library). This is the one to
  use if your project needs to demonstrate C++.

The frontend doesn't know or care which one is running — both expose the
identical `/api/solve`, `/api/compare`, and `/api/health` endpoints.

## How to run it

### 1. Backend — choose ONE of the two

**Option A: C++ backend (`backend-cpp/`)**

The socket code auto-detects the platform (`#ifdef _WIN32`) and uses Winsock
on Windows or POSIX sockets on Linux/macOS, so it builds natively everywhere
— no WSL needed.

*Linux / macOS:*
```bash
cd backend-cpp
make
./knights_tour_server
```

*Windows, directly in VS Code's terminal (PowerShell), using MinGW-w64:*

1. Install a Windows C++ compiler if you don't have one already — the
   easiest is [MSYS2](https://www.msys2.org/): download and run the
   installer, then in the "MSYS2 UCRT64" terminal it opens, run:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
   Then add `C:\msys64\ucrt64\bin` to your Windows PATH (search "Edit the
   system environment variables" → Environment Variables → edit `Path`),
   and restart VS Code so it picks up the new PATH.
2. Verify it's available — in VS Code's terminal:
   ```
   g++ --version
   ```
3. Build and run, from the `backend-cpp` folder:
   ```
   cd backend-cpp
   g++ -std=c++17 -O2 -pthread server.cpp algorithms.cpp json.cpp response_builder.cpp -lws2_32 -o knights_tour_server.exe
   .\knights_tour_server.exe
   ```

You should see `Knight's Tour C++ API listening on http://localhost:5000`.

**Option B: Node.js backend (`backend/`)**

```bash
cd backend
npm install
npm start
```

Both options run on **http://localhost:5000** and expose the same endpoints:

- `POST /api/solve` — `{ boardSize, startX, startY, algorithm }` → single algorithm's path + stats
- `POST /api/compare` — `{ boardSize, startX, startY }` → runs both algorithms, returns both results
- `GET /api/health` — health check

### 2. Frontend

```bash
cd frontend
npm install
npm run dev
```

Runs on **http://localhost:5173** (Vite dev server) and calls the backend at
`http://localhost:5000` by default. To point it elsewhere, set `VITE_API_BASE`
in a `.env` file inside `frontend/`.

## Using the app

1. Pick a board size (5×5 to 12×12) with the slider.
2. Click a square on the board to choose the knight's starting position.
3. Choose **Warnsdorff**, **Backtracking**, or **Compare both**.
4. Click **Run tour** — the backend solves it and the frontend animates the
   knight moving square by square, drawing its path as it goes.
5. Use the playback scrubber to replay, pause, or jump to any step, and the
   speed slider to control animation speed.
6. The **Analysis** panel on the right shows steps completed, success/failure,
   time taken, and (for backtracking) how many recursive calls it explored —
   useful for explaining the time-complexity trade-off in your report/demo.

## Notes for your report

- The backtracking implementation still orders its candidate moves using
  Warnsdorff's heuristic (fewest-options-first). This is a standard textbook
  optimization — plain unordered backtracking becomes impractically slow
  above roughly 6×6/7×7 boards, while this ordering lets it comfortably solve
  boards up to 12×12 in the demo.
- Warnsdorff's heuristic has no formal proof of correctness — it's a greedy
  heuristic that works extremely well in practice but can occasionally paint
  itself into a corner. The app reports `stuck` if that happens, which is
  itself a good talking point for a DSA presentation (heuristic vs.
  exhaustive search trade-offs).
- Board size is capped at 12×12 in the API purely so the backtracking demo
  stays responsive; the algorithm itself has no such limit.
- The C++ backend implements its own HTTP layer with raw POSIX sockets and a
  small hand-written JSON parser (`backend-cpp/json.h/.cpp`) instead of
  pulling in a web framework or JSON library. That's worth a paragraph in a
  report: parsing the request (splitting headers from body, reading
  `Content-Length` bytes) and parsing JSON (a small recursive-descent
  parser) are both genuine data-structure/parsing exercises in their own
  right, not just plumbing.
