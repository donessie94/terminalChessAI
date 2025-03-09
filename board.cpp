#include "board.h"
#include <ncurses.h>
#include <string>
#include <iostream>

BOARD::BOARD() {
    squareColor = {
         1, 0, 1, 0, 1, 0, 1, 0,
         0, 1, 0, 1, 0, 1, 0, 1,
         1, 0, 1, 0, 1, 0, 1, 0,
         0, 1, 0, 1, 0, 1, 0, 1,
         1, 0, 1, 0, 1, 0, 1, 0,
         0, 1, 0, 1, 0, 1, 0, 1,
         1, 0, 1, 0, 1, 0, 1, 0,
         0, 1, 0, 1, 0, 1, 0, 1
    };

    PIECES pieces;
    width = pieces.colSize * 8;
    heigth = pieces.rowSize * 8;

    // Initialize ncurses.
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);

    start_color();
    attron(A_BOLD);

    if (can_change_color()) {
        init_color(1, 380, 300, 200);
        init_color(2, 300, 100, 100);
        init_pair(1, COLOR_WHITE, 1);
        init_pair(2, COLOR_YELLOW, 1);
        init_pair(3, COLOR_WHITE, 2);
        init_pair(4, COLOR_YELLOW, 2);
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
    } else {
        init_pair(1, COLOR_WHITE, COLOR_MAGENTA);
        init_pair(2, COLOR_YELLOW, COLOR_MAGENTA);
        init_pair(3, COLOR_WHITE, COLOR_BLUE);
        init_pair(4, COLOR_YELLOW, COLOR_BLUE);
    }

    artDict = {
        {0,   pieces.emptyArt},
        {1,   pieces.pawnArt},
        {3,   pieces.knightArt},
        {5,   pieces.rookArt},
        {6,   pieces.bishopArt},
        {9,   pieces.queenArt},
        {127, pieces.kingArt}
    };
}

BOARD::~BOARD() {
    attroff(A_BOLD);
    endwin();
}

// Helper function to convert a piece code to its symbol.
// White pieces (positive) are uppercase; black pieces (negative) are lowercase.
// Example: Pawn: "P" (white) or "p" (black), Knight: "N"/"n", Bishop: "B"/"b", etc.
std::string BOARD::pieceCodeToSymbol(short code) {
    short absCode = std::abs(code);
    std::string symbol;
    if (absCode == 1)
        symbol = "P";
    else if (absCode == 3)
        symbol = "N";
    else if (absCode == 5)
        symbol = "R";
    else if (absCode == 6)
        symbol = "B";
    else if (absCode == 9)
        symbol = "Q";
    else if (absCode == 127)
        symbol = "K";
    else
        symbol = "?";
    if (code < 0) {
        for (auto &ch : symbol)
            ch = std::tolower(ch);
    }
    return symbol;
}

short BOARD::getRow(short index) {
    for (short k = 1; k <= 8; k++) {
        if (index < 8 * k)
            return k - 1;
    }
    return 7;
}
short BOARD::getCol(short index) {
    return index % 8;
}
std::string BOARD::indexToNotation(short index) {
    char file = 'a' + getCol(index);
    int rank = 8 - getRow(index);
    return std::string(1, file) + std::to_string(rank);
}

void BOARD::draw(const std::vector<short>& state) {
    PIECES piece;
    for (short i = 0; i < state.size() - 1; i++) {
        int colorPair = chooseColorPair(state[i], squareColor[i]);
        short key = std::abs(state[i]);
        const std::vector<std::string>& art = artDict[key];
        drawPieceAt(getRow(i) * piece.rowSize, getCol(i) * piece.colSize, art, colorPair);
    }
}
void BOARD::drawPieceAt(int startRow, int startCol, const std::vector<std::string>& art, int colorPair) {
    attron(COLOR_PAIR(colorPair));
    for (size_t i = 0; i < art.size(); i++) {
        mvprintw(startRow + i, startCol, "%s", art[i].c_str());
    }
    attroff(COLOR_PAIR(colorPair));
}

// In BOARD::drawInfo(), we print the move history to the right of the board.
// Each move is formatted in standard chess notation (e.g. "1. N e2xe4").
// It uses the full MoveInfo structure (which contains lastMove, movedPiece, and capturedPiece).
void BOARD::drawInfo(const std::vector<MoveInfo>& history) {
    int startRow = 0;
    int startCol = width + 1; // Print to the right of the board.
    mvprintw(startRow, startCol, "Move History:");
    startRow++;
    // For each move in the history, output the move number, piece symbol, and source/destination.
    for (size_t i = 0; i < history.size(); i++) {
        const auto &info = history[i];
        // Convert the source and destination board indices to chess notation.
        std::string sourceNotation = indexToNotation(info.lastMove.first);
        std::string destNotation = indexToNotation(info.lastMove.second);
        // Get the symbol for the moved piece.
        std::string pieceSymbol = pieceCodeToSymbol(info.movedPiece);
        // Use 'x' if a capture occurred, '-' otherwise.
        std::string separator = (info.capturedPiece != 0) ? "x" : "-";
        // Format the move string, e.g. "1. N e2xe4".
        std::string moveStr = std::to_string(i + 1) + ". " + pieceSymbol + " " + sourceNotation + separator + destNotation;
        mvprintw(startRow + i, startCol, "%s", moveStr.c_str());
    }
}

bool BOARD::clickInside(short colNum, short rowNum) {
    return (colNum < width && rowNum < heigth);
}
short BOARD::getClickedPieceIndex(const std::vector<short>& state, short colNum, short rowNum) {
    colNum = colNum / 11;
    rowNum = rowNum / 6;
    return rowNum * 8 + colNum;
}
void BOARD::highlight(const std::vector<short>& state, short index) {
    short key = std::abs(state[index]);
    const std::vector<std::string>& art = artDict[key];
    drawPieceAt(getRow(index) * 6, getCol(index) * 11, art, 5);
}
short BOARD::chooseColorPair(short pieceValue, bool isLightSquare) {
    if (isLightSquare)
        return (pieceValue > 0) ? 1 : 2;
    else
        return (pieceValue > 0) ? 3 : 4;
}