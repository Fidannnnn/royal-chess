# ♔ Royal Chess

A fully functional chess engine that runs in your terminal. Two players, or one player against an AI that actually knows what it's doing.

Built from scratch in C++ — no libraries, no shortcuts. The board is drawn with Unicode pieces and 24-bit ANSI colors, the AI uses minimax with alpha-beta pruning, and the whole thing is structured as a proper OOP project split across a clean module hierarchy.

---

## What's in here

The engine covers the full FIDE ruleset — castling (both sides), en passant, pawn promotion to any piece, check, checkmate, and stalemate. The AI searches 4 plies deep by default, uses piece-square tables for positional scoring, and orders moves to cut the search tree down as aggressively as possible.

On the display side, it uses 24-bit RGB color throughout — a classic wood and ivory board theme with muted highlights for selected squares, valid moves, the last move played, and king-in-check. The intro screen has an animated ASCII title and a piece parade. It's a terminal app but it doesn't look like one.

There's also a non-interactive pipe mode that activates automatically when stdin is a pipe — this is what the test script uses. You don't have to do anything special to trigger it; if you run the binary normally you get the game, if you pipe moves into it you get the test output.

---

## Getting started

You'll need `g++` with C++17 support. On Ubuntu that's just:

```bash
sudo apt install g++
```

On Windows use WSL, or MinGW with Windows Terminal for the colors to work properly.

**Build:**
```bash
make
```

**Play:**
```bash
./chess
```

**Clean up:**
```bash
make clean
```

---

## How to move

The game uses long algebraic notation — you type the from-square and the to-square together, no spaces:

| What you want | What you type |
|---|---|
| Move pawn from e2 to e4 | `e2e4` |
| Move knight from g1 to f3 | `g1f3` |
| Castle kingside | `O-O` |
| Castle queenside | `O-O-O` |
| Promote pawn to queen | `e7e8q` |
| Promote pawn to knight | `e7e8n` |

For promotion, `q` queen, `r` rook, `b` bishop, `n` knight. If you forget the suffix it defaults to queen.

Type `help` if you get stuck, `quit` to exit.

---

## Running the tests

The teacher's test script pipes move sequences into the binary and checks the output against expected board positions. It works automatically — just point it at the binary:

```bash
cd test
bash test-level.sh 1 ../chess   # basic piece movement
bash test-level.sh 2 ../chess   # check detection
bash test-level.sh 3 ../chess   # castling, en passant, promotion
bash test-level.sh 4 ../chess   # checkmate and stalemate
```

Results show up in the terminal and also get written to `test/test.log`.

---

## Project structure

The codebase is split into five layers. Each one only knows about the layers below it.

```
royal-chess/
├── PipeMode.hpp / .cpp     non-interactive test mode
├── Makefile
├── test/
└── src/
    ├── main.cpp
    ├── core/               Piece, Move, Board, GameState
    ├── engine/             MoveGenerator, AI
    ├── input/              InputParser
    ├── renderer/           Color, Terminal, BoardRenderer, Intro
    └── game/               Game (the main loop)
```

**core** is the foundation — plain data types and the board state. Nothing in here knows about rendering or user input.

**engine** is where the chess logic lives. `MoveGenerator` handles all legal move generation including the tricky cases (castling path safety, en passant timing). `AI` runs minimax with alpha-beta pruning and piece-square table evaluation.

**input** parses raw strings into moves. It handles the `O-O` aliases, promotion suffixes, and the two-line promotion protocol used by the test script.

**renderer** handles everything the user sees — colors, board drawing, the intro animation, and the side panel with move history and captured pieces.

**game** wires it all together. `Game` owns the main loop, talks to every other layer, and delegates as much as possible.

---

## The AI

The engine uses minimax search at depth 4. Alpha-beta pruning cuts the effective tree down significantly — in practice it visits roughly 2,000–15,000 nodes per move depending on the position, which keeps it under a second on most hardware.

Move ordering helps a lot: captures are sorted to the front of the move list by victim value, so the most promising lines get explored first and the pruning has more to work with.

Evaluation is material plus piece-square tables. Each piece type has a table that encodes where it prefers to be on the board — knights want the center, the king wants to stay behind pawns, pawns want to advance. It's simple but it produces reasonable play.

---

## Terminal requirements

The colors require a true-color terminal. On Linux or Mac, your default terminal almost certainly supports it. On Windows you need **Windows Terminal** — the old `cmd.exe` doesn't handle 24-bit ANSI and will show raw escape codes instead of colors.

WSL with Windows Terminal works perfectly.

---

## Team

This project was built by a team of 5 as part of an OOP2 course assignment. Each member owned one full vertical slice of the codebase — logic, data structures, and output — rather than being siloed into a single layer.

---

## License

Academic project. Not for redistribution.