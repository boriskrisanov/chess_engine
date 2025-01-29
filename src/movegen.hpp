#pragma once

#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "Piece.hpp"

struct MoveList
{
    void push_back(Move move)
    {
        moves[count++] = move;
    }

    void emplace_back(Square start, Square end, MoveFlag moveFlag)
    {
        moves[count++] = Move{start, end, moveFlag};
    }

    Move operator[](size_t i) const
    {
        return moves[i];
    }

private:
    std::array<Move, 218> moves{};
    uint8_t count = 0;
};

namespace movegen
{
    std::vector<Move> generateLegalMoves(Board& board);
    Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side);
    template <PieceKind Kind>
    Bitboard getPieceAttackingSquares(Bitboard allPieces, Bitboard pieces);
}
