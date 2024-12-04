#include "Move.hpp"
#include "Board.hpp"

Move::Move(Square start, Square end, MoveFlag flag)
{
    moveData |= start << 10;
    moveData |= end << 4;
    moveData |= static_cast<uint8_t>(flag);
}

Move::Move(const Board& board, std::string uciString)
{
    if (uciString.length() != 4 && uciString.length() != 5)
    {
        std::exit(1);
    }
    Square start = square::fromString(std::string{uciString.at(0)} + uciString.at(1));
    Square end = square::fromString(std::string{uciString.at(2)} + uciString.at(3));
    MoveFlag moveFlag = MoveFlag::None;
    // En Passant
    if (board[start].kind == PieceKind::PAWN && end == board.getEnPassantTargetSquare())
    {
        moveFlag = MoveFlag::EnPassant;
    }
    // Castling
    else if (board[start].kind == PieceKind::KING)
    {
        if (board.sideToMove == WHITE)
        {
            const Square c1 = square::fromString("c1");
            const Square g1 = square::fromString("g1");
            if (board.canWhiteShortCastle() && end == g1)
            {
                moveFlag = MoveFlag::ShortCastling;
            }
            else if (board.canWhiteLongCastle() && end == c1)
            {
                moveFlag = MoveFlag::LongCastling;
            }
        }
        else
        {
            const Square c8 = square::fromString("c8");
            const Square g8 = square::fromString("g8");
            if (board.canBlackShortCastle() && end == g8)
            {
                moveFlag = MoveFlag::ShortCastling;
            }
            else if (board.canBlackLongCastle() && end == c8)
            {
                moveFlag = MoveFlag::LongCastling;
            }
        }
    }
    // Promotion
    if (uciString.length() == 5)
    {
        switch (uciString.at(4))
        {
        case 'n':
            moveFlag = MoveFlag::PromotionKnight;
            break;
        case 'b':
            moveFlag = MoveFlag::PromotionBishop;
            break;
        case 'r':
            moveFlag = MoveFlag::PromotionRook;
            break;
        case 'q':
            moveFlag = MoveFlag::PromotionQueen;
            break;
        default:
            std::exit(1);
        }
    }
    moveData |= start << 10;
    moveData |= end << 4;
    moveData |= static_cast<uint8_t>(moveFlag);
}

uint8_t Move::start() const
{
    return (moveData & 0b1111110000000000) >> 10;
}

uint8_t Move::end() const
{
    return (moveData & 0b0000001111110000) >> 4;
}

MoveFlag Move::moveFlag() const
{
    return static_cast<MoveFlag>(moveData & 0b0000000000001111);
}

Move::operator std::string() const
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
