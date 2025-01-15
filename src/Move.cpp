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

std::string Move::getPgn(Board boardBeforeMove) const
{
    Board board = boardBeforeMove;
    std::string moveString;

    Piece movedPiece = board[start()];

    if (movedPiece.kind == PieceKind::PAWN)
    {
        if (capturedPiece.kind != PieceKind::NONE)
        {
            // First char of square will be the file
            moveString += square::toString(start())[0];
        }
    }
    else if (moveFlag() == MoveFlag::ShortCastling)
    {
        moveString.append("O-O");
    }
    else if (moveFlag() == MoveFlag::LongCastling)
    {
        moveString.append("O-O-O");
    }
    else
    {
        moveString += std::toupper(movedPiece.toString()[0]);
        /*
        Resolve ambiguous moves where multiple pieces of the same type can move to the same square. This is done
        by first generating all the legal moves in that position and checking if there are any moves with the same
        destination square. If there are, we iterate over them to check which position component (file or rank)
        is different, and add it to the move. If both the file and rank are the same (such as in the position
        8/k7/8/8/7Q/8/8/4Q1KQ, where 3 queens can move to e4), we append the full square after the letter of the
        moving piece.
         */
        std::vector<Move> movesToDestinationSquare;
        for (Move m : board.getLegalMoves())
        {
            if (board[m.start()].kind == movedPiece.kind && m.end() == end())
            {
                movesToDestinationSquare.push_back(m);
            }
        }
        if (movesToDestinationSquare.size() > 1)
        {
            std::vector<Square> otherStartPositions;
            for (Move m : movesToDestinationSquare)
            {
                if (m.start() != start())
                {
                    otherStartPositions.push_back(m.start());
                }
            }

            bool hasDifferentStartingRank = true;
            bool hasDifferentStartingFile = true;

            for (const Square startSquare : otherStartPositions)
            {
                if (square::rank(startSquare) == square::rank(startSquare))
                {
                    hasDifferentStartingRank = false;
                }
                if (square::file(startSquare) == square::file(startSquare))
                {
                    hasDifferentStartingFile = false;
                }
            }
            if (hasDifferentStartingRank)
            {
                moveString += square::toString(start())[1];
            }
            else if (hasDifferentStartingFile)
            {
                moveString += square::toString(start())[0];
            }
            else
            {
                moveString.append(square::toString(start()));
            }
        }
    }
    if (capturedPiece.kind != PieceKind::NONE)
    {
        moveString.append("x");
    }
    if (moveFlag() != MoveFlag::ShortCastling && moveFlag() != MoveFlag::LongCastling)
    {
        moveString.append(square::toString(end()));
    }
    if (isPromotion())
    {
        moveString.append("=");

        switch (moveFlag())
        {
        case MoveFlag::PromotionQueen:
            moveString.append("q");
            break;
        case MoveFlag::PromotionRook:
            moveString.append("r");
            break;
        case MoveFlag::PromotionBishop:
            moveString.append("b");
            break;
        case MoveFlag::PromotionKnight:
            moveString.append("n");
            break;
        default:
            break;
        }
    }
    if (board.isCheckmate(WHITE) || board.isCheckmate(BLACK))
    {
        moveString.append("#");
    }
    else if (board.isCheck())
    {
        // Comment from old code, not sure if this is still relevant
        // TODO: Fix + incorrectly being appended at the end of the full move
        moveString.append("+");
    }

    return moveString;
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
