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

const std::array<int, 64> blackKingEndgameWeights = {
    5, 4, 4, 4, 4, 4, 4, 5,
    4, 3, 3, 3, 3, 3, 3, 4,
    4, 3, 0, 0, 0, 0, 3, 4,
    4, 3, 0, 0, 0, 0, 3, 4,
    4, 3, 0, 0, 0, 0, 3, 4,
    4, 3, 0, 0, 0, 0, 3, 4,
    4, 3, 3, 3, 3, 3, 3, 4,
    5, 4, 4, 4, 4, 4, 4, 5
};

const std::array<int, 64> blackPawnOpeningWeights = switchOpeningWeightSide(whitePawnOpeningWeights);
const std::array<int, 64> blackKnightOpeningWeights = switchOpeningWeightSide(whiteKnightOpeningWeights);
const std::array<int, 64> blackKingOpeningWeights = switchOpeningWeightSide(whiteKingOpeningWeights);

// The weights are symmetrical around the center, so this simply inverts the sign
const std::array<int, 64> whiteKingEndgameWeights = switchOpeningWeightSide(blackKingEndgameWeights);

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

int whiteMaterial(const Board& board)
{
    using namespace pieceIndexes;
    int m = 0;
    m += std::popcount(board.bitboards[WHITE_PAWN]) * PAWN_VALUE;
    m += std::popcount(board.bitboards[WHITE_KNIGHT]) * KNIGHT_VALUE;
    m += std::popcount(board.bitboards[WHITE_BISHOP]) * BISHOP_VALUE;
    m += std::popcount(board.bitboards[WHITE_ROOK]) * ROOK_VALUE;
    m += std::popcount(board.bitboards[WHITE_QUEEN]) * QUEEN_VALUE;
    return m;
}

int blackMaterial(const Board& board)
{
    using namespace pieceIndexes;
    int m = 0;
    m += std::popcount(board.bitboards[BLACK_PAWN]) * PAWN_VALUE;
    m += std::popcount(board.bitboards[BLACK_KNIGHT]) * KNIGHT_VALUE;
    m += std::popcount(board.bitboards[BLACK_BISHOP]) * BISHOP_VALUE;
    m += std::popcount(board.bitboards[BLACK_ROOK]) * ROOK_VALUE;
    m += std::popcount(board.bitboards[BLACK_QUEEN]) * QUEEN_VALUE;
    return m;
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

int endgameEval(const Board& board)
{
    // TODO: Improve
    using namespace pieceIndexes;
    int eval = 0;

    const bool whiteHasSlidingPieces = (board.bitboards[WHITE_ROOK] | board.bitboards[WHITE_BISHOP] | board.bitboards[WHITE_QUEEN]) != 0;
    const bool blackHasSlidingPieces = (board.bitboards[BLACK_ROOK] | board.bitboards[BLACK_BISHOP] | board.bitboards[BLACK_QUEEN]) != 0;
    const int whiteKingPos = bitboards::getMSB(board.bitboards[WHITE_KING]);
    const int blackKingPos = bitboards::getMSB(board.bitboards[BLACK_KING]);

    eval += whiteKingEndgameWeights[whiteKingPos] * (blackHasSlidingPieces && !whiteHasSlidingPieces);
    eval += blackKingEndgameWeights[blackKingPos] * (whiteHasSlidingPieces && !blackHasSlidingPieces);

    const int distanceBetweenKings = abs(square::file(whiteKingPos) - square::file(blackKingPos)) + abs(square::rank(whiteKingPos) - square::rank(blackKingPos));

    eval -= (16 - distanceBetweenKings) * (blackHasSlidingPieces && !whiteHasSlidingPieces);
    eval += (16 - distanceBetweenKings) * (!blackHasSlidingPieces && whiteHasSlidingPieces);

    return eval;
}

double openingWeight(const Board& board)
{
    // This isn't very accurate, but it should be fine for now (ported from Java version)
    const double totalMaterial = whiteMaterial(board) + blackMaterial(board);
    return std::max(totalMaterial / 1024 - 2, 0.0);
}

int staticEval(const Board& board)
{
    int eval = 0;

    eval += whiteMaterial(board) - blackMaterial(board);
    eval += std::floor(static_cast<double>(openingSquareWeights(board)) * openingWeight(board));
    eval += std::floor(static_cast<double>(endgameEval(board)));

    return eval * (board.sideToMove == PieceColor::WHITE ? 1 : -1);
}

void printDebugEval(const Board& board)
{
    std::cout << "Opening weight: " << openingWeight(board) << "\n";
    std::cout << "Opening piece square table eval: " << openingSquareWeights(board) << "\n";
    std::cout << "Material imbalance: " << whiteMaterial(board) - blackMaterial(board) << "\n";
    std::cout << "Endgame eval: " << endgameEval(board) << "\n";
    std::cout << "Final eval: " << staticEval(board) * (board.sideToMove == PieceColor::WHITE ? 1 : -1) << "\n";
}
