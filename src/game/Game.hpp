#pragma once

#include "../core/GameState.hpp"
#include "../core/Move.hpp"
#include "../engine/AI.hpp"
#include "../renderer/BoardRenderer.hpp"
#include <string>
#include <vector>

// =============================================================================
//  GAME
//
//  Owns the main game loop and the top-level menu. Wires together every other
//  subsystem — GameState, MoveGenerator, AI, BoardRenderer, and InputParser —
//  without any of those systems needing to know about each other.
//
//  Responsibilities:
//    - Show the mode-selection menu and handle play-again flow
//    - Run a full game (PvP or PvAI) until checkmate, stalemate, or quit
//    - Maintain the captured-pieces trays and status message
//    - Compute per-frame RenderOptions (king-in-check square, etc.)
//    - Delegate all board output to BoardRenderer
//    - Delegate all AI moves to AI::bestMove()
//    - Delegate all input parsing to InputParser
//
//  Not responsible for:
//    - Anything inside the board (that's Board / GameState)
//    - Move legality (that's MoveGenerator)
//    - Rendering details (that's BoardRenderer)
// =============================================================================

class Game {
public:
    // Constructs a game with the AI at the default search depth
    Game();

    // Shows the main menu and runs games until the user chooses to quit.
    // This is the single entry point called from main().
    void run();

private:
    AI m_ai;  // owns the AI instance so depth and node count persist per game

    // Runs one full game. aiMode=true means black is played by the AI.
    void playGame(bool aiMode);

    // Displays the mode-selection menu and returns the user's choice:
    //   '1' = PvP,  '2' = PvAI,  'q' = quit
    char showMenu();

    // Handles the AI's turn: calls bestMove(), updates captured trays and status.
    // Returns the move the AI chose.
    Move doAiTurn(GameState& gs,
                  std::vector<std::string>& capturedB,
                  std::string& status);

    // Handles one human input cycle: reads a line, parses and validates it,
    // updates captures and status. Returns the matched legal move on success,
    // or an empty optional if the turn should be retried (bad input, help, quit).
    // Sets `quit` to true if the user typed quit/q/exit.
    bool doHumanTurn(GameState& gs,
                     const std::vector<Move>& legal,
                     std::vector<std::string>& capturedW,
                     std::vector<std::string>& capturedB,
                     std::string& status,
                     bool& quit,
                     Move& out);

    // Builds the RenderOptions for the current frame:
    // finds the king square when in check.
    static RenderOptions buildRenderOptions(const GameState& gs, bool inCheck);

    // Appends a captured piece's glyph to the right tray.
    // white=true means a white piece was captured (goes to capturedW).
    static void trackCapture(Piece captured, bool capturedIsWhite,
                              std::vector<std::string>& capturedW,
                              std::vector<std::string>& capturedB);
};