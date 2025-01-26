#pragma once

#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "Piece.hpp"

Bitboard generateAttackingSquares(Piece piece, Square position, const Board& board);
std::vector<Move> generateLegalMoves(Board& board);