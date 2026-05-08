#pragma once

#include "../core/Move.hpp"
#include "../core/GameState.hpp"
#include <string>
#include <vector>

//  INPUTPARSER
//
//  Converts raw user input strings into validated Move structs.
//  All methods are static — the class holds no state and is never instantiated.
//
//  The pipeline is two stages:
//
//    1. parse()        — turns a raw string into a Move (syntax only, no
//                        board awareness). Returns false on bad format.
//
//    2. matchLegal()   — finds the legal move in the current position that
//                        corresponds to the parsed move. Handles promotion
//                        color correction and castling row selection.
//                        Returns false if the move is not in the legal list.
//
//  Keeping the two stages separate makes both independently testable and
//  lets the Game loop give the user a precise error message for each failure
//  case ("bad format" vs "illegal move").

class InputParser {
public:
    InputParser() = delete;

    //  parse()
    //
    //  Parses a raw input string into a Move. Strips all whitespace first.
    //
    //  Accepted formats:
    //    Long algebraic:  "e2e4", "g1f3", "e7e8q" (promotion suffix optional)
    //    Castling:        "O-O", "0-0", "oo"       (kingside,  toC == 6)
    //                     "O-O-O", "0-0-0", "ooo"  (queenside, toC == 2)
    //
    //  Promotion suffix (5th character) is case-insensitive:
    //    q/Q = queen (default if omitted on promotion squares)
    //    r/R = rook
    //    b/B = bishop
    //    n/N or k/K = knight  ('k' is accepted as an alias to avoid confusion)
    //
    //  Does NOT validate whether the move is legal — that's matchLegal()'s job.
    //  Returns false if the string cannot be parsed into any recognized format.
    
    static bool parse(const std::string& input, Move& out);


    //  matchLegal()
    //
    //  Finds the legal move that corresponds to `parsed` in the given list.
    //  Writes the matched move into `found` and returns true on success.
    //
    //  Two adjustments are made during matching:
    //    - Castling: the king's row is inferred from the side to move (not
    //      stored in the parsed move, which only knows the destination column)
    //    - Promotion: if the user specified a piece, it is recolored to match
    //      the side to move. If they didn't (omitted suffix), defaults to queen.
    //
    //  Returns false if no legal move matches the parsed coordinates.
    static bool matchLegal(const std::vector<Move>& legal,
                           const Move& parsed,
                           bool whiteTurn,
                           Move& found);
};