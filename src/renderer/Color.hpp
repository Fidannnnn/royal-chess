#pragma once

#include <string_view>

//  COLOR.HPP — ANSI True-Color Escape Codes
//
//  All terminal styling in the project flows through this file.
//  Colors are 24-bit RGB (ESC[38;2;R;G;Bm foreground, ESC[48;2;R;G;Bm background),
//  which requires a true-color terminal: Windows Terminal, iTerm2, GNOME Terminal,
//  or any modern xterm-compatible emulator.
//
//  Why constexpr string_view instead of #define?
//    - Type-safe: they participate in overload resolution and work with templates
//    - Debuggable: they show up with their name in a debugger, not raw strings
//    - No macro pitfalls: no accidental token pasting, no scope leakage
//    - Zero runtime cost: the compiler treats them identically to string literals
//
//  The FG / BG macros are kept for the one place they're still needed —
//  building escape sequences from runtime RGB values in the intro animation.
//  Everything else uses the named constants below.

// Text style codes 

inline constexpr std::string_view RESET = "\033[0m";
inline constexpr std::string_view BOLD = "\033[1m";
inline constexpr std::string_view DIM = "\033[2m";
inline constexpr std::string_view ITALIC = "\033[3m";
inline constexpr std::string_view UNDERLINE = "\033[4m";
inline constexpr std::string_view BLINK = "\033[5m";

// Parameterized macros for runtime RGB values (intro gradient only).
// Prefer the named constants below for everything else
#define FG(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define BG(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"

//  Classic Wood & Ivory Theme
//
//  Palette inspired by traditional tournament sets: warm parchment on light
//  squares, aged mahogany on dark ones. Every color in the UI is drawn from
//  the same warm brown/gold/ivory family so nothing feels out of place.
//
//  Highlights are intentionally muted — they need to be readable without
//  pulling the eye away from the pieces themselves.

// Board squares 

inline constexpr std::string_view CLR_LIGHT_SQ = "\033[48;2;210;190;150m"; // parchment ivory
inline constexpr std::string_view CLR_DARK_SQ = "\033[48;2;135;88;45m"; // aged mahogany

// Square highlights

inline constexpr std::string_view CLR_HL_SQ     = "\033[48;2;168;195;130m"; // selected — soft sage
inline constexpr std::string_view CLR_HL_MOVE   = "\033[48;2;205;185;100m"; // valid target — warm amber
inline constexpr std::string_view CLR_HL_CHECK  = "\033[48;2;185;75;65m";   // king in check — terracotta
inline constexpr std::string_view CLR_HL_LAST   = "\033[48;2;145;170;185m"; // last move — dusty slate

// Piece foreground colors 
//  Both sides use the same outline glyphs (♙♘♗♖♕♔).
//  Color alone distinguishes them, which keeps rendering consistent
//  across terminals that vary in how they handle filled Unicode chess glyphs.

inline constexpr std::string_view CLR_W_PIECE   = "\033[38;2;250;245;225m"; // warm ivory white
inline constexpr std::string_view CLR_B_PIECE   = "\033[38;2;90;52;18m";    // warm dark brown

// UI chrome 
//  Everything stays within the warm walnut palette.
//  No cool grays, no neon accents

inline constexpr std::string_view CLR_TITLE      = "\033[38;2;190;150;75m";       // antique gold
inline constexpr std::string_view CLR_BORDER      = "\033[38;2;90;58;25m";        // dark walnut
inline constexpr std::string_view CLR_INFO      = "\033[38;2;155;132;100m";       // parchment text
inline constexpr std::string_view CLR_WHITE_SIDE      = "\033[38;2;225;210;178m"; // light-side label
inline constexpr std::string_view CLR_BLACK_SIDE      = "\033[38;2;105;78;50m";   // dark-side label
inline constexpr std::string_view CLR_CAPTURED      = "\033[38;2;170;135;88m";    // captured piece tray
inline constexpr std::string_view CLR_MOVE_NUM      = "\033[38;2;118;148;105m";   // move numbers
inline constexpr std::string_view CLR_COORD      = "\033[38;2;95;65;30m";         // rank/file labels
inline constexpr std::string_view CLR_STATUS_OK      = "\033[38;2;108;152;88m";   // OK — forest green
inline constexpr std::string_view CLR_STATUS_WARN      = "\033[38;2;188;142;65m"; // warning — warm ochre
inline constexpr std::string_view CLR_STATUS_ERR      = "\033[38;2;178;78;65m";   // error — muted terracotta
