#pragma once

#include "Piece.hpp"
#include <string>

// =============================================================================
//  MOVE
//
//  Represents a single half-move (ply). Stores source and destination squares
//  plus flags for the three special move types that need extra handling during
//  applyMove(): castling, en passant, and promotion.
//
//  Design notes:
//    - Kept as a struct (all public) because it's pure data with no invariants
//      to protect — any layer that constructs a Move is responsible for setting
//      the fields correctly.
//    - toAlgebraic() lives here rather than in a separate formatter because it
//      only depends on the Move's own fields and the Piece helpers; pulling it
//      out would add indirection for no benefit.
//    - The score field is a scratch space used only by the AI's move-ordering
//      step. It has no meaning outside the search.
// =============================================================================

struct Move {
    int fromR = 0, fromC = 0;  // source square (row 0 = rank 8, row 7 = rank 1)
    int toR   = 0, toC   = 0;  // destination square

    // Promotion target piece — EMPTY for all non-promotion moves.
    // Set to the correct color by the move generator / input parser.
    Piece promotion = EMPTY;

    bool isCastle    = false;  // king moves two squares; rook is repositioned by applyMove()
    bool isEnPassant = false;  // capturing pawn is not on the same square as the captured pawn

    // Scratch field for move ordering inside minimax — captures are scored
    // by the value of the victim so they bubble to the front of the list.
    // Not meaningful outside the AI search.
    int score = 0;

    // -------------------------------------------------------------------------
    //  toAlgebraic()
    //
    //  Serializes the move to long algebraic notation (coordinate notation):
    //    - Normal move:    "e2e4"
    //    - Promotion:      "e7e8q"  (lowercase piece letter appended)
    //
    //  Rows are stored internally as 0=rank8 .. 7=rank1, so rank is computed
    //  as (7 - row) when converting to the 1-based display rank.
    // -------------------------------------------------------------------------
    std::string toAlgebraic() const {
        std::string s;
        s += static_cast<char>('a' + fromC);
        s += static_cast<char>('1' + (7 - fromR));
        s += static_cast<char>('a' + toC);
        s += static_cast<char>('1' + (7 - toR));

        if (promotion != EMPTY) {
            // promoChars is indexed by the white-side piece value (1–6),
            // so black promotions are normalized via toWhite() first
            const char promoChars[] = "?pnbrqk";
            s += promoChars[toWhite(promotion)];
        }
        return s;
    }
};