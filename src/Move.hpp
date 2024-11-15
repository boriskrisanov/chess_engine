#pragma once

#include "Piece.hpp"
#include "Square.hpp"

class Board;

enum class MoveFlag : uint8_t
{
    None,
    EnPassant,
    ShortCastling,
    LongCastling,
    PromotionKnight,
    PromotionBishop,
    PromotionRook,
    PromotionQueen
};

class Move
{
public:
    Move(Square start, Square end, MoveFlag flag);
    Move(const Board& board, std::string uciString);
    Move() = default;
    Square start() const;
    Square end() const;
    MoveFlag moveFlag() const;
    int score = 0;

    Piece capturedPiece;

    bool isPromotion() const
    {
        return moveFlag() == MoveFlag::PromotionKnight || moveFlag() == MoveFlag::PromotionBishop || moveFlag() ==
            MoveFlag::PromotionRook || moveFlag() == MoveFlag::PromotionQueen;
    }

    explicit operator std::string() const
    {
        std::string s = square::toString(start()) + square::toString(end());
        if (isPromotion())
        {
            switch (moveFlag())
            {
            case MoveFlag::PromotionKnight:
                s += "n";
                break;
            case MoveFlag::PromotionBishop:
                s += "b";
                break;
            case MoveFlag::PromotionRook:
                s += "r";
                break;
            case MoveFlag::PromotionQueen:
                s += "q";
                break;
            default:
                break;
            }
        }
        return s;
    }

private:
    uint16_t moveData = 0;
    // 000000 000000 0000
    // 6 bits - start index
    // 6 bits - end index
    // 4 bits - flag
};
