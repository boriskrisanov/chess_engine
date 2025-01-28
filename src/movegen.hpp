#pragma once

#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "Piece.hpp"

struct MoveList
{
    std::array<Move, 255> moves{};
    uint8_t count = 0;
};

namespace movegen
{
    std::vector<Move> generateLegalMoves(Board& board);
    Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side);
    template <PieceKind Kind>
    Bitboard getPieceAttackingSquares(Bitboard allPieces, Bitboard pieces);
}