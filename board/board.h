#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <unordered_map>
#include "../pieces/pieces.h"
#include "../utils/moveinfo.h"
#include <ncurses.h>

struct BOARD {
    // Data members.
    std::vector<bool> squareColor;  // Light (true) or dark (false) squares.
    short width;    // In character columns.
    short heigth;   // In character rows.
    std::unordered_map<short, std::vector<std::string>> artDict;

    BOARD();
    ~BOARD();

    // Board coordinate helpers.
    short getRow(short index);
    short getCol(short index);
    std::string indexToNotation(short index);
    std::string pieceCodeToSymbol(short code);

    // Drawing functions: versions that accept a WINDOW* so that output goes to that window.
    void draw(const std::vector<short>& state, WINDOW* win);
    void drawPieceAt(int startRow, int startCol, const std::vector<std::string>& art, int colorPair, WINDOW* win);
    void drawInfo(const std::vector<MoveInfo>& history, WINDOW* win);
    void drawUndoButton(WINDOW* win);

    // Input helpers.
    bool clickInside(short colNum, short rowNum);
    short getClickedPieceIndex(const std::vector<short>& state, short colNum, short rowNum);
    void highlight(const std::vector<short>& state, short index, WINDOW* win);
    bool clickUndoButton(short colNum, short rowNum);

    // Color helper.
    short chooseColorPair(short pieceValue, bool isLightSquare);






    void testDraw(WINDOW* win) {
        mvwprintw(win, 1, 1, "Test: Board drawing works!");
        wrefresh(win);
    }
};

#endif // BOARD_H