#include "board.h"
#include <ncurses.h>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>

BOARD::BOARD() {
    // Initialize square colors for an 8x8 board.
    squareColor = {
         true, false, true, false, true, false, true, false,
         false, true, false, true, false, true, false, true,
         true, false, true, false, true, false, true, false,
         false, true, false, true, false, true, false, true,
         true, false, true, false, true, false, true, false,
         false, true, false, true, false, true, false, true,
         true, false, true, false, true, false, true, false,
         false, true, false, true, false, true, false, true
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
        init_color(3, 800, 800, 800);
        init_color(4, 1000, 1000, 0);   //bright yellow
        init_color(5, 1000, 1000, 1000); //bright white
        init_color(6, 200, 900, 200); //bright green
        init_color(7, 1000, 200, 200); //bright red
        init_pair(1, 5, 1);
        init_pair(2, 4, 1);
        init_pair(3, 5, 2);
        init_pair(4, 4, 2);
        init_pair(5, 6, COLOR_BLACK);
        init_pair(6, 7, 3);
    } else {

        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_BLUE);
        init_pair(4, COLOR_YELLOW, COLOR_BLUE);
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        init_pair(6, COLOR_RED, COLOR_BLACK);
    }

    // Set up the art dictionary.
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
        std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
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

short BOARD::chooseColorPair(short pieceValue, bool isLightSquare) {
    if (isLightSquare)
        return (pieceValue > 0) ? 1 : 2;
    else
        return (pieceValue > 0) ? 3 : 4;
}

// Draw the board to the specified window.
void BOARD::draw(const std::vector<short>& state, WINDOW* win) {
    PIECES piece;
    for (short i = 0; i < state.size() - 1; i++) {
        int colorPair = chooseColorPair(state[i], squareColor[i]);
        short key = std::abs(state[i]);
        const std::vector<std::string>& art = artDict[key];
        int row = getRow(i) * piece.rowSize;
        int col = getCol(i) * piece.colSize;
        drawPieceAt(row, col, art, colorPair, win);
    }
    wrefresh(win);
}

void BOARD::drawPieceAt(int startRow, int startCol, const std::vector<std::string>& art, int colorPair, WINDOW* win) {
    wattron(win, COLOR_PAIR(colorPair));
    for (size_t i = 0; i < art.size(); i++) {
        mvwprintw(win, startRow + i, startCol, "%s", art[i].c_str());
    }
    wattroff(win, COLOR_PAIR(colorPair));
}

void BOARD::drawUndoButton(WINDOW* win) {
    PIECES piece;
    std::vector<std::string> undoButton = piece.undoButton;
    int buttonHeight = undoButton.size();
    int startRow = heigth - buttonHeight;
    int startCol = width + 1;
    wattron(win, COLOR_PAIR(6));
    for (size_t i = 0; i < undoButton.size(); i++) {
        mvwprintw(win, startRow + i, startCol, "%s", undoButton[i].c_str());
    }
    wattroff(win, COLOR_PAIR(6));
    wrefresh(win);
}

void BOARD::drawInfo(const std::vector<MoveInfo>& history, WINDOW* win) {
    int startRow = 0;
    int startCol = width + 1;
    mvwprintw(win, startRow, startCol, "Move History:");
    startRow++;
    for (size_t i = 0; i < history.size(); i++) {
        const auto &info = history[i];
        std::string sourceNotation = indexToNotation(info.lastMove.first);
        std::string destNotation = indexToNotation(info.lastMove.second);
        std::string pieceSymbol = pieceCodeToSymbol(info.movedPiece);
        std::string separator = (info.capturedPiece != 0) ? "x" : "-";
        std::string moveStr = std::to_string(i + 1) + ". " + pieceSymbol + " " + sourceNotation + separator + destNotation;
        mvwprintw(win, startRow + i, startCol, "%s", moveStr.c_str());
    }
    wrefresh(win);
}

bool BOARD::clickInside(short colNum, short rowNum) {
    return (colNum < width && rowNum < heigth);
}

short BOARD::getClickedPieceIndex(const std::vector<short>& state, short colNum, short rowNum) {
    // Convert pixel/character coordinates to board index.
    // (You may need to adjust these divisors based on your drawing dimensions.)
    short col = colNum / 11;
    short row = rowNum / 6;
    return row * 8 + col;
}

bool BOARD::clickUndoButton(short colNum, short rowNum) {
    PIECES pieces;
    std::vector<std::string> undoButton = pieces.undoButton;
    int buttonHeight = undoButton.size();
    int buttonWidth = undoButton[0].size();
    int startRow = heigth - buttonHeight;
    int startCol = width + 1;
    return (colNum >= startCol && colNum < startCol + buttonWidth &&
            rowNum >= startRow && rowNum < startRow + buttonHeight);
}

void BOARD::highlight(const std::vector<short>& state, short index, WINDOW* win) {
    short key = std::abs(state[index]);
    const std::vector<std::string>& art = artDict[key];
    // For highlighting, we use color pair 5.
    drawPieceAt(getRow(index) * 6, getCol(index) * 11, art, 5, win);
    wrefresh(win);
}