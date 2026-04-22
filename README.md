# TetriC

A Windows console Tetris clone written in C with local and network play modes.

## Features
- Local mode with single-player and two-player gameplay.
- Network mode with single, two-player, and multiplayer gameplay.
- Server/client socket flow for online matches.
- Multiplayer matches continue until one winner remains.
- Easy, Medium, and Hard difficulty presets.
- Next-piece preview.
- Level-based speed increases over time.
- Soft drop and hard drop scoring.
- Per-player high scores saved to local text files.

## Menu Flow
1. Choose `Local` or `Network`.
2. Local mode offers `Single Player` or `Two Players`.
3. Network mode offers `Single`, `Two Player`, or `Multiplayer`.
4. For online two-player or multiplayer, choose `Server` or `Client`.
5. A multiplayer server chooses the total player count.

## Controls

Single Player and Local Player 1:
- Move: `A` / `D`
- Rotate: `W`
- Soft drop: `S`
- Hard drop: `Z`
- Pause: `P`
- Quit: `Q`

Local Player 2:
- Move: Left / Right Arrow
- Rotate: Up Arrow
- Soft drop: Down Arrow
- Hard drop: Space

Online:
- Control your own board with `WASD/Z`.
- Arrow keys and `Space` also work for your local online board.
- `P` toggles pause for the online session.
- `Q` leaves the current match.

## Scoring and Leveling
- 1 line: `100 x level`
- 2 lines: `300 x level`
- 3 lines: `500 x level`
- 4 or more lines: `800 x level`
- Soft drop: `+1` per row
- Hard drop: `+2` per row

## Build (Windows)

### MinGW (GCC)
```bash
gcc -std=c11 -O2 -Wall -Wextra -pedantic -o TetriC.exe *.c -lws2_32
```

If your GCC toolchain is older and does not expose `getaddrinfo` by default, this project now defines the required Windows version macros in [TetriC.h](/d:/SPL1/TetriC/TetriC.h). If you are building an older copy, add `-D_WIN32_WINNT=0x0600`.

### MSVC (Developer Command Prompt)
```bat
cl /O2 /W3 /Fe:TetriC.exe *.c
```

## Run
```bat
TetriC.exe
```
