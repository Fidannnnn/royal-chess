#include "InputParser.hpp"

#include <algorithm>
#include <cctype>

// =============================================================================
//  INPUTPARSER  —  Implementation
// =============================================================================

bool InputParser::parse(const std::string& input, Move& out) {
    // Strip all whitespace so "e2 e4" and "e2e4" both work
    std::string s = input;
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

    if (s.empty()) return false;

    // -- Castling shorthands --------------------------------------------------
    // Only the destination column is recorded here — the king's row is filled
    // in by matchLegal() once we know whose turn it is.
    if (s == "O-O"   || s == "0-0"   || s == "oo")  {
        out = Move{};
        out.isCastle = true;
        out.toC = 6;  // kingside
        return true;
    }
    if (s == "O-O-O" || s == "0-0-0" || s == "ooo") {
        out = Move{};
        out.isCastle = true;
        out.toC = 2;  // queenside
        return true;
    }

    // -- Long algebraic notation ----------------------------------------------
    // Minimum length is 4 characters: fromFile + fromRank + toFile + toRank
    if (s.size() < 4) return false;

    // File letters: 'a'=0 .. 'h'=7
    // Rank digits:  '1'=row 7 .. '8'=row 0  (board is stored rank-8-first)
    int fc = s[0] - 'a';
    int fr = 8 - (s[1] - '0');
    int tc = s[2] - 'a';
    int tr = 8 - (s[3] - '0');

    // Reject anything outside the 8×8 grid
    if (fc<0||fc>7 || fr<0||fr>7 || tc<0||tc>7 || tr<0||tr>7) return false;

    out = Move{};
    out.fromR = fr;  out.fromC = fc;
    out.toR   = tr;  out.toC   = tc;

    // -- Optional promotion suffix (5th character) ----------------------------
    // We store white-side piece values here; matchLegal() recolors as needed.
    // 'k' is accepted as an alias for knight since 'n' can be ambiguous for
    // players coming from descriptive notation.
    if (s.size() >= 5) {
        char pc = static_cast<char>(tolower(static_cast<unsigned char>(s[4])));
        out.promotion = (pc == 'n' || pc == 'k') ? W_KNIGHT :
                        (pc == 'b')              ? W_BISHOP :
                        (pc == 'r')              ? W_ROOK   : W_QUEEN;
    }

    return true;
}

bool InputParser::matchLegal(const std::vector<Move>& legal,
                             const Move& parsed,
                             bool whiteTurn,
                             Move& found) {
    // -- Castling -------------------------------------------------------------
    // Match by destination column (6 = kingside, 2 = queenside) and the
    // king's starting row for the side to move.
    if (parsed.isCastle) {
        const int kingRow = whiteTurn ? 7 : 0;
        for (const Move& m : legal) {
            if (m.isCastle && m.toC == parsed.toC && m.fromR == kingRow) {
                found = m;
                return true;
            }
        }
        return false;
    }

    // -- Normal move ----------------------------------------------------------
    // Match by all four coordinates. The first legal move that agrees on
    // from/to is the one we want — there can't be two legal moves between
    // the same squares unless they differ only by promotion piece, and the
    // promotion correction below handles that.
    for (const Move& m : legal) {
        if (m.fromR != parsed.fromR || m.fromC != parsed.fromC) continue;
        if (m.toR   != parsed.toR   || m.toC   != parsed.toC)   continue;

        found = m;

        // Promotion piece correction:
        //   - User specified a piece → recolor it to the correct side
        //   - User omitted the suffix on a promotion move → default to queen
        if (m.promotion != EMPTY) {
            if (parsed.promotion != EMPTY) {
                // toWhite() gives us the white-side base; +6 converts to black
                Piece whiteSide = toWhite(parsed.promotion);
                found.promotion = whiteTurn
                    ? whiteSide
                    : static_cast<Piece>(whiteSide + 6);
            } else {
                found.promotion = whiteTurn ? W_QUEEN : B_QUEEN;
            }
        }

        return true;
    }

    return false;
}