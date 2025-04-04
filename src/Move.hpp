#pragma once

#include "MoveFlag.hpp"
#include "Piece.hpp"
#include "Square.hpp"
#include <cstdint>


class Board;

class Move
{
  public:
    Move(Square start, Square end, MoveFlag flag);
    Move(const Board &board, const std::string &uciString);
    Move() = default;
    Square start() const;
    Square end() const;
    MoveFlag moveFlag() const;
    std::string getPgn(Board boardBeforeMove) const;
    int score = 0;

    Piece capturedPiece;

    bool isPromotion() const
    {
        return moveFlag() == MoveFlag::PromotionKnight || moveFlag() == MoveFlag::PromotionBishop || moveFlag() == MoveFlag::PromotionRook || moveFlag() == MoveFlag::PromotionQueen;
    }

    bool isInvalid() const
    {
        return moveData == 0;
    }

    explicit operator std::string() const;

    bool operator==(const Move rhs) const
    {
        return moveData == rhs.moveData;
    }

  private:
    uint16_t moveData = 0;
    // 000000 000000 0000
    // 6 bits - start index
    // 6 bits - end index
    // 4 bits - flag
};
