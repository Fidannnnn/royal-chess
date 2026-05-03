#pragma once

#include <string>

// =============================================================================
//  PIECE DEFINITIONS
//
//  Pieces are encoded as a plain enum so they can be used as array indices
//  directly. White occupies 1–6, black 7–12, and 0 means empty.
//
//  This layout has two useful properties:
//    - Color is determined by a single range check (no bit flags needed)
//    - Converting between sides is just arithmetic: black = white + 6
// =============================================================================

enum Piece {
    EMPTY    = 0,
    W_PAWN   = 1, W_KNIGHT = 2, W_BISHOP = 3,
    W_ROOK   = 4, W_QUEEN  = 5, W_KING   = 6,
    B_PAWN   = 7, B_KNIGHT = 8, B_BISHOP = 9,
    B_ROOK   = 10, B_QUEEN = 11, B_KING  = 12
};

// =============================================================================
//  DATA TABLES
//  Indexed by Piece value (0–12), so lookups are a single array access.
// =============================================================================

// Both sides share the same outline glyphs — color is handled by the renderer.
// Filled black glyphs (♟♞ etc.) render inconsistently across terminals,
// so we avoid them entirely and rely on foreground color to distinguish sides.
inline const std::string PIECE_UNICODE[] = {
    "  ",
    " ♙", " ♘", " ♗", " ♖", " ♕", " ♔",  // white
    " ♙", " ♘", " ♗", " ♖", " ♕", " ♔"   // black (tinted dark brown by renderer)
};

// Human-readable names for status messages and notation
inline const std::string PIECE_NAMES[] = {
    "",
    "Pawn", "Knight", "Bishop", "Rook", "Queen", "King",
    "Pawn", "Knight", "Bishop", "Rook", "Queen", "King"
};

// Single-character FEN-style notation (uppercase = white, lowercase = black)
inline const char PIECE_CHARS[] = {
    ' ', 'P', 'N', 'B', 'R', 'Q', 'K',
    'p', 'n', 'b', 'r', 'q', 'k'
};

// Centipawn material values used by the AI evaluator.
// The king's value is intentionally enormous so it always dominates
// the material sum — this prevents the search from trading kings.
inline const int PIECE_VALUE[] = {
    0,
    100, 320, 330, 500, 900, 20000,   // white
    100, 320, 330, 500, 900, 20000    // black
};

// =============================================================================
//  HELPER PREDICATES
//
//  Kept as free functions rather than methods because Piece is a plain enum,
//  not a class. Marking them inline avoids ODR violations across translation
//  units that include this header.
// =============================================================================

// Returns true if p belongs to the white side
inline bool isWhite(Piece p) { return p >= W_PAWN && p <= W_KING; }

// Returns true if p belongs to the black side
inline bool isBlack(Piece p) { return p >= B_PAWN && p <= B_KING; }

// Returns true if the square is unoccupied
inline bool isEmpty(Piece p) { return p == EMPTY; }

// Returns true if both pieces belong to the same side.
// Used in move generation to reject moves that land on friendly squares.
inline bool sameColor(Piece a, Piece b) {
    return (isWhite(a) && isWhite(b)) || (isBlack(a) && isBlack(b));
}

// Returns the white-side equivalent of any piece.
// No-op if p is already white or EMPTY. Used to index into piece-square tables
// that are stored only for white and mirrored for black.
inline Piece toWhite(Piece p) {
    return static_cast<Piece>(p > 6 ? p - 6 : p);
}