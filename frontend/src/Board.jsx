import React from "react";

const FILES = "abcdefghijkl".split("");

// Renders an n x n board, the trail walked so far (up to `revealCount` moves),
// and the knight glyph at the current step.
export default function Board({
  n,
  path,
  revealCount,
  accent = "var(--accent)",
  label,
  onSquareClick,
  startSquare,
  interactive = false,
}) {
  const visited = path.slice(0, revealCount);
  const visitedMap = new Map(visited.map(([x, y], i) => [`${x},${y}`, i]));
  const current = visited[visited.length - 1];

  const size = n <= 6 ? 56 : n <= 8 ? 46 : n <= 10 ? 38 : 32;
  const boardPx = size * n;

  const points = visited
    .map(([x, y]) => `${x * size + size / 2},${y * size + size / 2}`)
    .join(" ");

  return (
    <div className="board-wrap">
      {label && <div className="board-label">{label}</div>}
      <div
        className="board"
        style={{ width: boardPx, height: boardPx }}
      >
        {Array.from({ length: n }).map((_, y) =>
          Array.from({ length: n }).map((_, x) => {
            const isDark = (x + y) % 2 === 1;
            const key = `${x},${y}`;
            const order = visitedMap.get(key);
            const isStart = startSquare && startSquare[0] === x && startSquare[1] === y;
            return (
              <div
                key={key}
                className={`square ${isDark ? "dark" : "light"} ${
                  interactive ? "clickable" : ""
                }`}
                style={{
                  width: size,
                  height: size,
                  left: x * size,
                  top: y * size,
                }}
                onClick={() => interactive && onSquareClick && onSquareClick(x, y)}
                title={`${FILES[x]}${n - y}`}
              >
                {order !== undefined && (
                  <span
                    className="order-badge"
                    style={{
                      color: order === visited.length - 1 ? "var(--bg)" : "var(--text-dim)",
                      background:
                        order === visited.length - 1 ? accent : "transparent",
                    }}
                  >
                    {order + 1}
                  </span>
                )}
                {isStart && order === undefined && (
                  <span className="start-dot" style={{ background: accent }} />
                )}
              </div>
            );
          })
        )}

        <svg
          className="path-overlay"
          width={boardPx}
          height={boardPx}
          viewBox={`0 0 ${boardPx} ${boardPx}`}
        >
          {visited.length > 1 && (
            <polyline
              points={points}
              fill="none"
              stroke={accent}
              strokeWidth={Math.max(2, size * 0.06)}
              strokeLinecap="round"
              strokeLinejoin="round"
              opacity={0.85}
            />
          )}
        </svg>

        {current && (
          <div
            className="knight"
            style={{
              width: size,
              height: size,
              left: current[0] * size,
              top: current[1] * size,
              fontSize: size * 0.6,
            }}
          >
            ♞
          </div>
        )}
      </div>
    </div>
  );
}
