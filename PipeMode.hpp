#pragma once

// =============================================================================
//  PIPEMODE
//
//  Non-interactive test mode activated when stdin is a pipe (not a terminal).
//  Reads moves line by line, applies legal ones silently, skips illegal ones,
//  and on /quit prints the board state + game result in the format expected
//  by the teacher's test script:
//
//    wR,wN,wB,wQ,wK,wB,wN,wR,..., ?-?
//
//  Board encoding (64 squares, rank 8 first, comma-separated):
//    empty square  -> ""   (nothing between commas)
//    white piece   -> wP wN wB wR wQ wK
//    black piece   -> bP bN bB bR bQ bK
//
//  Game result:
//    ?-?       ongoing or /quit before game over
//    1-0       checkmate, black to move has no moves
//    0-1       checkmate, white to move has no moves
//    1/2-1/2   stalemate
// =============================================================================

class PipeMode {
public:
    PipeMode() = delete;

    // Runs the full pipe-mode game loop. Returns 0 on clean exit.
    static int run();
};