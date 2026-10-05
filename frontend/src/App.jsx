import React, { useEffect, useRef, useState } from "react";
import Board from "./Board.jsx";
import "./App.css";

const API_BASE = import.meta.env.VITE_API_BASE || "http://localhost:5000";

export default function App() {
  const [boardSize, setBoardSize] = useState(8);
  const [start, setStart] = useState([0, 0]);
  const [mode, setMode] = useState("warnsdorff"); // "warnsdorff" | "backtracking" | "compare"
  const [loading, setLoading] = useState(false);
  const [errorMsg, setErrorMsg] = useState(null);

  const [result, setResult] = useState(null); // single-algorithm result
  const [compareResult, setCompareResult] = useState(null); // {warnsdorff, backtracking}

  const [reveal, setReveal] = useState(0);
  const [speed, setSpeed] = useState(120); // ms per step
  const [playing, setPlaying] = useState(false);
  const timerRef = useRef(null);

  const activePath = result?.path || compareResult?.warnsdorff?.path || [];
  const maxSteps = mode === "compare"
    ? Math.max(compareResult?.warnsdorff?.path?.length || 0, compareResult?.backtracking?.path?.length || 0)
    : (result?.path?.length || 0);

  useEffect(() => {
    if (!playing) return;
    if (reveal >= maxSteps) {
      setPlaying(false);
      return;
    }
    timerRef.current = setTimeout(() => setReveal((r) => r + 1), speed);
    return () => clearTimeout(timerRef.current);
  }, [playing, reveal, speed, maxSteps]);

  async function runSolve() {
    setLoading(true);
    setErrorMsg(null);
    setReveal(0);
    setPlaying(false);
    setResult(null);
    setCompareResult(null);

    try {
      const endpoint = mode === "compare" ? "/api/compare" : "/api/solve";
      const body = {
        boardSize,
        startX: start[0],
        startY: start[1],
        ...(mode !== "compare" ? { algorithm: mode } : {}),
      };
      const res = await fetch(`${API_BASE}${endpoint}`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(body),
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || "Request failed");

      if (mode === "compare") {
        setCompareResult(data);
      } else {
        setResult(data);
      }
      setPlaying(true);
    } catch (err) {
      setErrorMsg(
        err.message === "Failed to fetch"
          ? `Can't reach the backend at ${API_BASE}. Is the Express server running?`
          : err.message
      );
    } finally {
      setLoading(false);
    }
  }

  function handleSquareClick(x, y) {
    if (loading) return;
    setStart([x, y]);
    setResult(null);
    setCompareResult(null);
    setReveal(0);
    setPlaying(false);
  }

  function resetPlayback() {
    setReveal(0);
    setPlaying(true);
  }

  const n = boardSize;

  return (
    <div className="page">
      <header className="hero">
        <div className="eyebrow">DSA PROJECT · GRAPH TRAVERSAL &amp; BACKTRACKING</div>
        <h1>Knight&rsquo;s Tour</h1>
        <p className="subtitle">
          A knight must visit every square of an n&times;n board exactly once.
          This board plays out two classic approaches move by move —
          <strong> Warnsdorff&rsquo;s heuristic</strong> (greedy, fast) and
          <strong> Backtracking</strong> (exhaustive, guaranteed) — so you can watch
          the algorithm think, not just read about it.
        </p>
      </header>

      <main className="layout">
        <section className="board-section">
          {mode !== "compare" ? (
            <Board
              n={n}
              path={activePath}
              revealCount={reveal}
              accent="var(--accent)"
              startSquare={start}
              interactive={!result}
              onSquareClick={handleSquareClick}
            />
          ) : (
            <div className="compare-boards">
              <Board
                n={n}
                path={compareResult?.warnsdorff?.path || []}
                revealCount={Math.min(reveal, compareResult?.warnsdorff?.path?.length || 0)}
                accent="var(--accent-2)"
                label="Warnsdorff's Heuristic"
                startSquare={start}
                interactive={!compareResult}
                onSquareClick={handleSquareClick}
              />
              <Board
                n={n}
                path={compareResult?.backtracking?.path || []}
                revealCount={Math.min(reveal, compareResult?.backtracking?.path?.length || 0)}
                accent="var(--danger)"
                label="Backtracking"
                startSquare={start}
                interactive={!compareResult}
                onSquareClick={handleSquareClick}
              />
            </div>
          )}

          {(result || compareResult) && (
            <div className="playback">
              <button onClick={resetPlayback} className="ghost-btn">
                ⟲ Replay
              </button>
              <button onClick={() => setPlaying((p) => !p)} className="ghost-btn">
                {playing ? "⏸ Pause" : "▶ Play"}
              </button>
              <input
                type="range"
                min="0"
                max={maxSteps}
                value={reveal}
                onChange={(e) => {
                  setPlaying(false);
                  setReveal(Number(e.target.value));
                }}
              />
              <label className="speed-label">
                speed
                <input
                  type="range"
                  min="20"
                  max="400"
                  step="20"
                  value={420 - speed}
                  onChange={(e) => setSpeed(420 - Number(e.target.value))}
                />
              </label>
            </div>
          )}

          {!result && !compareResult && (
            <p className="hint">Click a square above to choose the knight's starting position.</p>
          )}
        </section>

        <aside className="panel">
          <div className="panel-block">
            <div className="panel-title">Board size</div>
            <input
              type="range"
              min="5"
              max="12"
              value={boardSize}
              onChange={(e) => {
                const v = Number(e.target.value);
                setBoardSize(v);
                setStart(([x, y]) => [Math.min(x, v - 1), Math.min(y, v - 1)]);
                setResult(null);
                setCompareResult(null);
                setReveal(0);
                setPlaying(false);
              }}
            />
            <div className="mono-readout">{n} × {n}</div>
          </div>

          <div className="panel-block">
            <div className="panel-title">Algorithm</div>
            <div className="algo-toggle">
              {[
                ["warnsdorff", "Warnsdorff"],
                ["backtracking", "Backtracking"],
                ["compare", "Compare both"],
              ].map(([val, lbl]) => (
                <button
                  key={val}
                  className={`chip ${mode === val ? "chip-active" : ""}`}
                  onClick={() => {
                    setMode(val);
                    setResult(null);
                    setCompareResult(null);
                    setReveal(0);
                    setPlaying(false);
                  }}
                >
                  {lbl}
                </button>
              ))}
            </div>
          </div>

          <button className="run-btn" onClick={runSolve} disabled={loading}>
            {loading ? "Solving…" : "Run tour"}
          </button>

          {errorMsg && <div className="error-box">{errorMsg}</div>}

          <div className="panel-block analysis">
            <div className="panel-title">Analysis</div>
            {!result && !compareResult && (
              <div className="mono-readout dim">Run a tour to see stats.</div>
            )}
            {result && <StatLines r={result} n={n} />}
            {compareResult && (
              <div className="compare-stats">
                <StatLines r={compareResult.warnsdorff} n={n} accent="var(--accent-2)" />
                <StatLines r={compareResult.backtracking} n={n} accent="var(--danger)" />
              </div>
            )}
          </div>

          <div className="panel-block note">
            <div className="panel-title">How it works</div>
            <p>
              <strong>Warnsdorff:</strong> at every step, move to the reachable
              square with the fewest onward moves. Greedy, no backtracking,
              O(n²) — but not guaranteed to finish.
            </p>
            <p>
              <strong>Backtracking:</strong> try a move, recurse, and undo it if
              every continuation fails. Always finds a tour if one exists, at
              the cost of exponential worst-case time (bounded here with a
              node/time limit on larger boards).
            </p>
          </div>
        </aside>
      </main>

      <footer className="footer">
        Backend: Express (Node.js) running the algorithms in{" "}
        <code>backend/algorithms/</code> · Frontend: React + Vite
      </footer>
    </div>
  );
}

function StatLines({ r, n, accent }) {
  return (
    <div className="stat-lines" style={accent ? { borderColor: accent } : undefined}>
      <div className="stat-row">
        <span>algorithm</span>
        <span style={{ color: accent || "var(--accent)" }}>{r.algorithm}</span>
      </div>
      <div className="stat-row">
        <span>status</span>
        <span className={r.success ? "ok" : "fail"}>
          {r.success ? "complete tour" : r.timedOut ? "timed out" : "stuck"}
        </span>
      </div>
      <div className="stat-row">
        <span>squares visited</span>
        <span>{r.steps} / {n * n}</span>
      </div>
      {"nodesExplored" in r && (
        <div className="stat-row">
          <span>nodes explored</span>
          <span>{r.nodesExplored.toLocaleString()}</span>
        </div>
      )}
      <div className="stat-row">
        <span>time</span>
        <span>{r.timeMs} ms</span>
      </div>
    </div>
  );
}
