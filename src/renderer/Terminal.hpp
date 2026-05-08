#pragma once

#include "../renderer/Color.hpp"
#include <string>
#include <string_view>


//  TERMINAL
//
//  Thin wrapper around the raw ANSI escape sequences that control the terminal
//  itself — cursor visibility, position, screen clearing, sleep, and centered
//  text output.
//
//  All methods are static: Terminal holds no state of its own, it's purely a
//  namespace with a type attached. A static class is preferable to a free
//  namespace here because it can be forward-declared, mocked in tests, and
//  passed as a template parameter if needed down the line.
//
//  Nothing outside this class should write raw cursor/screen escape codes.
//  Color escape codes (CLR_*) live in Color.hpp and are used by the renderer
//  directly — those are data, not control sequences.

class Terminal {
public:
    Terminal() = delete;  // purely static — never instantiated

    // -- Screen control -------------------------------------------------------

    // Clears the entire screen and moves the cursor to the home position (1,1)
    static void clearScreen();

    // Moves the cursor to the given row and column (1-based, top-left = 1,1)
    static void moveCursor(int row, int col);

    // Hides the blinking cursor — call during rendering to prevent flicker
    static void hideCursor();

    // Restores the cursor — always call this before the program exits
    static void showCursor();

    // Timing 

    // Blocks for the given number of milliseconds.
    // Used by the intro animation for paced output.
    static void sleepMs(int ms);

    // Text output

    /*
     * Prints `text` centered within `width` terminal columns.
     * Padding is added on the left only (right padding isn't needed for
     * single-line centered output). If `color` is provided, the text is
     * wrapped in that escape sequence and reset afterwards.
     *
     * `color` should be one of the CLR_* constants from Color.hpp,
     * or an empty string_view to print with no color override.
     */
    static void printCentered(const std::string& text,
                               int width,
                               std::string_view color = "");
};