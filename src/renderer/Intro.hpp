#pragma once

//  INTRO
//
//  Plays the animated title screen shown once at program startup.
//  Isolated into its own class so main.cpp stays clean and the animation
//  can be skipped or replaced without touching anything else.
//
//  All methods are static — Intro holds no state and is never instantiated.

class Intro {
public:
    Intro() = delete;

    // Clears the screen, animates the ASCII title and piece parade,
    // then returns with the cursor restored.
    static void play();

private:
    // Streams the ASCII art title line by line with a gold-to-amber gradient
    static void drawTitle();

    // Streams the two-row piece parade (dark pieces on ivory, light on mahogany)
    // with a short delay between each glyph
    static void drawParade();
};