#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <string>
#include <unordered_map>
#include "pieces.h"   // Assumes this header defines your PIECES struct
#include "moveinfo.h"

struct BOARD {
    // Data members.
    std::vector<bool> squareColor;
    short width;
    short heigth;
    std::unordered_map<short, std::vector<std::string>> artDict;

    BOARD();
    ~BOARD();

    // Board coordinate helpers.
    short getRow(short index);
    short getCol(short index);
    std::string indexToNotation(short index);
    std::string pieceCodeToSymbol(short code);

    // Drawing functions.
    void draw(const std::vector<short>& state);
    void drawPieceAt(int startRow, int startCol, const std::vector<std::string>& art, int colorPair);
    void drawInfo(const std::vector<MoveInfo>& history);
    void drawUndoButton();

    // Input helpers.
    bool clickInside(short colNum, short rowNum);
    short getClickedPieceIndex(const std::vector<short>& state, short colNum, short rowNum);
    void highlight(const std::vector<short>& state, short index);
    bool clickUndoButton(short colNum, short rowNum);

    // Color helper.
    short chooseColorPair(short pieceValue, bool isLightSquare);
};

#endif // BOARD_H