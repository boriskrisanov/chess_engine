#pragma once

#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "Piece.hpp"

void generatePseudoLegalMoves(Piece piece, Square position, const Board& board, std::vector<Move>& moves);
void generateLegalMoves(Piece piece, Square position, Board& board, std::vector<Move>& moves);
Bitboard generateAttackingSquares(Piece piece, Square position, const Board& board);