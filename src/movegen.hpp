#pragma once

#include "MoveList.hpp"
#include "Move.hpp"
#include "Piece.hpp"
#include "bitboards.hpp"

class Board;

namespace movegen
{
    MoveList generateLegalMoves(Board& board);
    Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side);
    template <PieceKind Kind>
    Bitboard getPieceAttackingSquares(Bitboard allPieces, Bitboard pieces);
}
