# TetriC

A Windows console Tetris clone written in C with single-player and local two-player modes.

## Features
- Single-player and local two-player gameplay.
- Easy, Medium, and Hard difficulty presets.
- Next-piece preview.
- Level-based speed increases over time.
- Soft drop and hard drop scoring.
- Per-difficulty high scores saved to local text files.

## Controls

Single Player:
- Move: A / D
- Rotate: W
- Soft drop: S
- Hard drop: Z
- Pause: P
- Quit: Q

Two Players:
- Player 1: A / D / W / S, Hard drop: Z
- Player 2: Arrow keys, Hard drop: Space
- Pause: P
- Quit: Q

## Scoring and Leveling
- Line clears (per clear, multiplied by current level):
  - 1 line: 100
  - 2 lines: 300
  - 3 lines: 500
  - 4+ lines: 800
- Soft drop: +1 per row.
- Hard drop: +2 per row.
- Speed increases on a timer based on the selected difficulty until a minimum speed is reached.

## Build (Windows)

This project uses Windows console APIs and is intended for Windows.

### MinGW (GCC)
```bash
gcc -std=c11 -O2 -o TetriC.exe *.c -lws2_32
```

### MSVC (Developer Command Prompt)
```bat
cl /O2 /W3 /Fe:TetriC.exe *.c
```

## Run
```bat
TetriC.exe
```

## High Scores
High scores are stored in plain text files in the project directory:
- `highscore_easy_p1.txt`, `highscore_easy_p2.txt`
- `highscore_medium_p1.txt`, `highscore_medium_p2.txt`
- `highscore_hard_p1.txt`, `highscore_hard_p2.txt`
