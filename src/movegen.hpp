#pragma once

#include "Move.hpp"
#include "MoveList.hpp"
#include "Piece.hpp"
#include "bitboards.hpp"

class Board;

namespace movegen
{
struct EdgeDistance
{
    uint8_t WEST;
    uint8_t EAST;
    uint8_t NORTH;
    uint8_t SOUTH;
    uint8_t NORTHWEST;
    uint8_t NORTHEAST;
    uint8_t SOUTHWEST;
    uint8_t SOUTHEAST;
};

MoveList generateLegalMoves(Board &board);
Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side);
template <PieceKind Kind>
Bitboard getPieceAttackingSquares(Bitboard allPieces, Bitboard pieces);
std::array<EdgeDistance, 64> getEdgeDistances();

std::array<Bitboard, 64> getBishopBlockerMasks();
std::array<Bitboard, 64> getRookBlockerMasks();
std::vector<Bitboard> possibleBlockerPositions(Bitboard blockerMask);
} // namespace movegen
