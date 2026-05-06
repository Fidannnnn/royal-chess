#pragma once

#include "../core/GameState.hpp"
#include "../renderer/Color.hpp"
#include <string>
#include <vector>

// =============================================================================
//  BOARDRENDERER
//
//  Owns all terminal output related to the chess board: the board grid,
//  square highlights, piece glyphs, side panel, captured pieces tray,
//  move history, and input prompt.
//
//  The class is stateless — every piece of information it needs is passed
//  in through draw(). This makes it trivial to call from anywhere without
//  worrying about stale internal state.
//
//  All methods are static. draw() is the only public entry point;
//  everything else is a private helper that breaks the render into
//  logical sections.
// =============================================================================

// -----------------------------------------------------------------------------
//  RenderOptions
//
//  Carries the per-frame highlight state that the Game loop computes
//  before each call to draw(). Kept as a plain struct — it's pure data
//  passed from the game loop to the renderer with no behavior of its own.
// -----------------------------------------------------------------------------
struct RenderOptions {
    int selR = -1, selC = -1;                        // currently selected square (-1 = none)
    std::vector<std::pair<int,int>> validMoves;       // legal destinations for the selected piece
    int checkR = -1, checkC = -1;                     // king's square when in check (-1 = not in check)
};

class BoardRenderer {
public:
    BoardRenderer() = delete;

    // -------------------------------------------------------------------------
    //  draw()
    //
    //  Clears the screen and renders the full board UI:
    //    - Title box
    //    - Board grid with pieces and all highlight layers
    //    - Side panel (player labels, material balance, move counter, status)
    //    - Captured pieces tray (one row per side)
    //    - Move history (last 10 half-moves)
    //    - Input prompt
    //
    //  Parameters:
    //    gs         — current game position (read-only)
    //    opt        — highlight state for this frame
    //    capturedW  — white pieces taken by black (displayed next to black's side)
    //    capturedB  — black pieces taken by white (displayed next to white's side)
    //    statusMsg  — pre-colored status string shown in the side panel
    //    aiMode     — controls "AI Engine" vs "Player 2" label
    // -------------------------------------------------------------------------
    static void draw(const GameState& gs,
                     const RenderOptions& opt,
                     const std::vector<std::string>& capturedW,
                     const std::vector<std::string>& capturedB,
                     const std::string& statusMsg,
                     bool aiMode);

private:
    // Returns the background color escape sequence for square (r,c).
    // Priority order: check highlight > selected > valid move > last move > normal.
    static std::string_view squareBG(const GameState& gs, int r, int c,
                                     const RenderOptions& opt);

    // Renders one row of the board grid including the piece glyph
    // and the side panel element that aligns with that row.
    static void drawRow(int r, const GameState& gs, const RenderOptions& opt,
                        const std::vector<std::string>& capturedW,
                        const std::vector<std::string>& capturedB,
                        const std::string& statusMsg, bool aiMode);

    // Side panel helpers — each corresponds to one row of the board
    static void drawPanelRow(int r, const GameState& gs,
                             const std::vector<std::string>& capturedW,
                             const std::vector<std::string>& capturedB,
                             const std::string& statusMsg, bool aiMode);

    static void drawFileLabels();   // "  a   b   c  ..." above and below the board
    static void drawTitle();
    static void drawHistory(const GameState& gs);
};