#include "miscellanous.h"

std::vector<PIECE_TYPE> allPieceTypes()
{
    return {
        PIECE_TYPE::PAWN,
        PIECE_TYPE::ROOK,
        PIECE_TYPE::KNIGHT,
        PIECE_TYPE::BISHOP,
        PIECE_TYPE::QUEEN,
        PIECE_TYPE::KING,
    };
}

std::unordered_map<int, std::pair<int, char>> buildPosToNotation()
{
    std::unordered_map<int, std::pair<int,char>> dict;
    dict.reserve(64);

    // We will have a mapping like this:
    //   index  0 → (rank=1, file='a')   → "a1"
    //   index  1 → (rank=1, file='b')   → "b1"
    //   ...
    //   index  7 → (rank=1, file='h')   → "h1"
    //   index  8 → (rank=2, file='a')   → "a2"
    //   ...
    //   index 63 → (rank=8, file='h')   → "h8"

    for (int i = 0; i < 64; ++i) {
        int rank = 1 + (i / 8);             // i=0..7 → rank=1; i=8..15 → rank=2; …; i=56..63 → rank=8
        char file = static_cast<char>('a' + (i % 8));
        dict.emplace(i, std::make_pair(rank, file));
    }
    return dict;
}

void POSITION::setPosition(int newPos)
{
    if (newPos < 0 || newPos > 63) {
            throw std::out_of_range("POSITION index must be between 0 and 63");
        }
        index = newPos;
        // Lookup file/rank
        auto [r, f] = posToNotation.at(index);
        rank = r;
        file = f;

        // Convert file char → 0..7
        int fileIdx = file - 'a';      // 'a'→0, 'b'→1, … 'h'→7

        // Diagonal (NW→SE): a8=1, a7=2…a1=8, b1=9…h1=15
        diagonal = fileIdx + (8 - rank) + 1;

        // Anti-diagonal (NE→SW): a1=1…a8=8, b8=9…h8=15
        antiDiagonal = fileIdx + rank;
}

POSITION::POSITION(int pos) { setPosition(pos); }

std::string POSITION::toAlgebraicNotation() const { return std::string{file} + std::to_string(rank); }

MOVE::MOVE( POSITION fromPos, POSITION toPos, PIECE_TYPE pt,
        COLOR color, bool capture, PIECE_TYPE capturedType,
        bool promotion, PIECE_TYPE promoType,
        bool check, bool checkmate)
        : from(fromPos), to(toPos), pieceType(pt),
          pieceColor(color), isCapture(capture), capturedPieceType(capturedType),
          isPromotion(promotion), promotionType(promoType),
          isCheck(check), isCheckmate(checkmate) {}
