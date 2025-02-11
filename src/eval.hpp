#pragma once
#include <cstdint>

#include "Piece.hpp"
#include "search.hpp"

constexpr int PAWN_VALUE = 100;
constexpr int KNIGHT_VALUE = 300;
constexpr int BISHOP_VALUE = 350;
constexpr int ROOK_VALUE = 500;
constexpr int QUEEN_VALUE = 900;

uint16_t pieceValue(PieceKind kind);
int staticEval(const Board& board);
void printDebugEval(const Board& board);
