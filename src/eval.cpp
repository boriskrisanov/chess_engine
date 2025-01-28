#include "eval.hpp"

#include <iostream>
#include <array>
#include <cmath>
#include "Board.hpp"
#include "search.hpp"

std::array<int, 64> switchOpeningWeightSide(std::array<int, 64> weights)
{
    // Reverse ranks (assuming weights are symmetrical)
    // Array is copied so this is fine
    std::reverse(weights.begin(), weights.end());
    for (int& weight : weights)
    {
        weight *= -1;
    }
    return weights;
}

const std::array<int, 64> whitePawnOpeningWeights = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 4, 4, 3, 0, 0,
    0, 2, 3, 4, 4, 3, 2, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const std::array<int, 64> whiteKnightOpeningWeights = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 3, 3, 0, 0, 0,
    0, 0, 3, 2, 2, 3, 0, 0,
    0, 0, 4, 2, 2, 4, 0, 0,
    0, 0, 0, 2, 2, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

const std::array<int, 64> whiteKingOpeningWeights = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 5, 2, 0, 0, 2, 5, 5
};

const std::array<int, 64> blackPawnOpeningWeights = switchOpeningWeightSide(whitePawnOpeningWeights);
const std::array<int, 64> blackKnightOpeningWeights = switchOpeningWeightSide(whiteKnightOpeningWeights);
const std::array<int, 64> blackKingOpeningWeights = switchOpeningWeightSide(whiteKingOpeningWeights);

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

int materialImbalance(const Board& board)
{
    using namespace pieceIndexes;
    int eval = 0;

    eval += std::popcount(board.bitboards[WHITE_PAWN]) * PAWN_VALUE;
    eval += std::popcount(board.bitboards[WHITE_KNIGHT]) * KNIGHT_VALUE;
    eval += std::popcount(board.bitboards[WHITE_BISHOP]) * BISHOP_VALUE;
    eval += std::popcount(board.bitboards[WHITE_ROOK]) * ROOK_VALUE;
    eval += std::popcount(board.bitboards[WHITE_QUEEN]) * QUEEN_VALUE;

    eval -= std::popcount(board.bitboards[BLACK_PAWN]) * PAWN_VALUE;
    eval -= std::popcount(board.bitboards[BLACK_KNIGHT]) * KNIGHT_VALUE;
    eval -= std::popcount(board.bitboards[BLACK_BISHOP]) * BISHOP_VALUE;
    eval -= std::popcount(board.bitboards[BLACK_ROOK]) * ROOK_VALUE;
    eval -= std::popcount(board.bitboards[BLACK_QUEEN]) * QUEEN_VALUE;

    return eval;
}

int openingSquareWeights(const Board& board)
{
    int eval = 0;

    for (int i = 0; i < 64; i++)
    {
        if (board[i].kind() == PieceKind::PAWN)
        {
            eval += board[i].color() == PieceColor::WHITE ? whitePawnOpeningWeights[i] : blackPawnOpeningWeights[i];
        }
        else if (board[i].kind() == PieceKind::KNIGHT)
        {
            eval += board[i].color() == PieceColor::WHITE ? whiteKnightOpeningWeights[i] : blackKnightOpeningWeights[i];
        }
        else if (board[i].kind() == PieceKind::KING)
        {
            eval += board[i].color() == PieceColor::WHITE ? whiteKingOpeningWeights[i] : blackKingOpeningWeights[i];
        }
    }

    return eval;
}

double openingWeight(const Board& board)
{
    // This isn't very accurate, but it should be fine for now (ported from Java version)
    using namespace pieceIndexes;
    double totalMaterial = 0;

    totalMaterial += std::popcount(board.bitboards[WHITE_PAWN]) * PAWN_VALUE;
    totalMaterial += std::popcount(board.bitboards[WHITE_KNIGHT]) * KNIGHT_VALUE;
    totalMaterial += std::popcount(board.bitboards[WHITE_BISHOP]) * BISHOP_VALUE;
    totalMaterial += std::popcount(board.bitboards[WHITE_ROOK]) * ROOK_VALUE;
    totalMaterial += std::popcount(board.bitboards[WHITE_QUEEN]) * QUEEN_VALUE;

    totalMaterial -= std::popcount(board.bitboards[BLACK_PAWN]) * PAWN_VALUE;
    totalMaterial -= std::popcount(board.bitboards[BLACK_KNIGHT]) * KNIGHT_VALUE;
    totalMaterial -= std::popcount(board.bitboards[BLACK_BISHOP]) * BISHOP_VALUE;
    totalMaterial -= std::popcount(board.bitboards[BLACK_ROOK]) * ROOK_VALUE;
    totalMaterial -= std::popcount(board.bitboards[BLACK_QUEEN]) * QUEEN_VALUE;

    return std::max((totalMaterial / 1024) - 2, 0.0);
}

int staticEval(const Board& board)
{
    int eval = 0;

    eval += materialImbalance(board);
    eval += std::floor(static_cast<double>(openingSquareWeights(board)) * openingWeight(board));

    return eval * (board.sideToMove == PieceColor::WHITE ? 1 : -1);
}

void printDebugEval(const Board& board)
{
    std::cout << "Opening weight: " << openingWeight(board) << "\n";
    std::cout << "Opening piece square table eval: " << openingSquareWeights(board) << "\n";
    std::cout << "Material imbalance: " << materialImbalance(board) << "\n";
    std::cout << "Final eval: " << staticEval(board) * (board.sideToMove == PieceColor::WHITE ? 1 : -1) << "\n";
}
