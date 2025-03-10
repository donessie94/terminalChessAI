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
    //use_default_colors();  // Allow transparency: -1 will mean “use terminal’s default”
    attron(A_BOLD);

    if (can_change_color()) {
        // We can redefine colors here:
        init_color(1, 380, 300, 200);  // Some custom color
        init_color(2, 300, 100, 100);  // Another custom color
        init_color(3, 800, 800, 800);  // Another custom color

        // Now define color pairs using these custom colors.
        init_pair(1, COLOR_WHITE, 1);   // White text on custom color 1
        init_pair(2, COLOR_YELLOW, 1); // Yellow text on custom color 1
        init_pair(3, COLOR_WHITE, 2);  // White text on custom color 2
        init_pair(4, COLOR_YELLOW, 2); // Yellow text on custom color 2
        init_pair(5, COLOR_GREEN, COLOR_BLACK);
        init_pair(6, COLOR_RED, 3);    // Red text on custom color 3
    } else {
        // Fallback: do NOT call init_color().
        // Use only built-in color constants like COLOR_BLACK, COLOR_BLUE, etc.

        // For example, define color pairs with standard backgrounds:
        init_pair(1, COLOR_WHITE, COLOR_BLACK);   // White text on black
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Yellow text on black
        init_pair(3, COLOR_WHITE, COLOR_BLUE);    // White text on blue
        init_pair(4, COLOR_YELLOW, COLOR_BLUE);   // Yellow text on blue
        init_pair(5, COLOR_GREEN, COLOR_BLACK);   // Green text on black
        init_pair(6, COLOR_RED, COLOR_BLACK);     // Red text on black

        //         // Fallback if terminal does not support changing colors:
        // init_pair(1, COLOR_WHITE, -1);   // White piece on default background (light square)
        // init_pair(2, COLOR_MAGENTA, -1); // Black piece on default background (light square)
        // init_pair(3, COLOR_WHITE, COLOR_BLUE);  // White piece on dark square
        // init_pair(4, COLOR_MAGENTA, COLOR_BLUE); // Black piece on dark square
        // init_pair(5, COLOR_GREEN, -1);           // For other elements
        // init_pair(6, COLOR_RED, -1);             // For the UNDO button
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

void BOARD::drawUndoButton() {
    PIECES piece;
    std::vector<std::string> undoButton = piece.undoButton;

    // Determine where to draw the button.
    // For instance, at the bottom right: starting row = heigth - buttonHeight,
    // and starting column = width + 1 (to the right of the board).
    int buttonHeight = undoButton.size();
    int startRow = heigth - buttonHeight;
    int startCol = width + 1;

    // Draw each line of the UNDO button using color pair 6.
    attron(COLOR_PAIR(6));
    for (size_t i = 0; i < undoButton.size(); i++) {
        mvprintw(startRow + i, startCol, "%s", undoButton[i].c_str());
    }
    attroff(COLOR_PAIR(6));
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
// Returns true if the click at (colNum, rowNum) is within the bounds of the UNDO button.
// The UNDO button is drawn at the bottom right of the board:
//   - Its top-left corner is at (width + 1, heigth - buttonHeight)
//   - Its dimensions are determined by the size of pieces.undoButton.
bool BOARD::clickUndoButton(short colNum, short rowNum) {
    // Create a temporary PIECES instance to access the undo button art.
    PIECES pieces;
    std::vector<std::string> undoButton = pieces.undoButton;

    // Determine the dimensions of the button.
    int buttonHeight = undoButton.size();
    int buttonWidth = undoButton[0].size(); // Assuming all lines have equal length.

    // Calculate the top-left coordinates of the button:
    int startRow = heigth - buttonHeight;
    int startCol = width + 1;

    // Check if the click falls within the button's bounding box.
    if (colNum >= startCol && colNum < startCol + buttonWidth &&
        rowNum >= startRow && rowNum < startRow + buttonHeight) {
        return true;
    }
    return false;
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