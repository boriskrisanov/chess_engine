#pragma once
#include <chrono>

#include "Board.hpp"
#include "Move.hpp"
#include "Piece.hpp"

struct DebugStats
{
    uint64_t positionsEvaluated = 0;
};

extern DebugStats debugStats;

struct SearchResult
{
    PieceColor sideToMove;
    Move bestMove;
    int eval;

    SearchResult(PieceColor sideToMove, Move bestMove, int eval)
        : sideToMove(sideToMove), bestMove(bestMove), eval(eval)
    {
    }

    double standardEval() const {
        // Divide by 100 to convert centipawns to pawns
        return static_cast<double>(sideToMove == BLACK ? eval * -1 : eval) / 100;
    }
};

SearchResult bestMove(Board& board, uint8_t depth);
SearchResult timeLimitedSearch(Board& board, std::chrono::milliseconds timeLimit);
