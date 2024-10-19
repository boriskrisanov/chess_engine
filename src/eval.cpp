#include "eval.hpp"

#include <iostream>

uint16_t pieceValue(PieceKind kind)
{
    switch (kind)
    {
    case PieceKind::PAWN:
        return PAWN_VALUE;
    case PieceKind::KNIGHT:
        return KNIGHT_VALUE;
    case PieceKind::BISHOP:
        return BISHOP_VALUE;
    case PieceKind::ROOK:
        return ROOK_VALUE;
    case PieceKind::QUEEN:
        return QUEEN_VALUE;
    default:
        return 0;
    }
}

int staticEval(const Board& board)
{
    int eval = 0;

    eval += std::popcount(board.whitePawns) * PAWN_VALUE;
    eval += std::popcount(board.whiteKnights) * KNIGHT_VALUE;
    eval += std::popcount(board.whiteBishops) * BISHOP_VALUE;
    eval += std::popcount(board.whiteRooks) * ROOK_VALUE;
    eval += std::popcount(board.whiteQueens) * QUEEN_VALUE;

    eval -= std::popcount(board.blackPawns) * PAWN_VALUE;
    eval -= std::popcount(board.blackKnights) * KNIGHT_VALUE;
    eval -= std::popcount(board.blackBishops) * BISHOP_VALUE;
    eval -= std::popcount(board.blackRooks) * ROOK_VALUE;
    eval -= std::popcount(board.blackQueens) * QUEEN_VALUE;

    return eval * (board.sideToMove == WHITE ? 1 : -1);
}
