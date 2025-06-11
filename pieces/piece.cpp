#include "piece.h"

PIECE::PIECE(PIECE_TYPE t, COLOR c, POSITION pos)
        : type(t), color(c), position(pos){}

std::string PIECE::getTypeAsString() const
{
    switch (type) {
            case PIECE_TYPE::PAWN:   return "PAWN";
            case PIECE_TYPE::ROOK:   return "ROOK";
            case PIECE_TYPE::KNIGHT: return "KNIGHT";
            case PIECE_TYPE::BISHOP: return "BISHOP";
            case PIECE_TYPE::QUEEN:  return "QUEEN";
            case PIECE_TYPE::KING:   return "KING";
            case PIECE_TYPE::EMPTY:  return "EMPTY";
            default:                 return "UNKNOWN";
        }
}

std::string PIECE::getColorAsString() const { return (color == COLOR::WHITE) ? "WHITE" : "BLACK"; }

std::string PIECE::getPositionAsString() const { return position.toAlgebraicNotation(); }
