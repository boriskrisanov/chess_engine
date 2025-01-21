#pragma once
#include <chrono>

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
    DebugStats debugStats;

    SearchResult(PieceColor sideToMove, Move bestMove, int eval, DebugStats debugStats)
        : sideToMove(sideToMove), bestMove(bestMove), eval(eval), debugStats(debugStats)
    {
    }

    double standardEval() const {
        // Divide by 100 to convert centipawns to pawns
        return static_cast<double>(sideToMove == PieceColor::BLACK ? eval * -1 : eval) / 100;
    }
};

SearchResult bestMove(Board& board, uint8_t depth);
SearchResult timeLimitedSearch(Board& board, std::chrono::milliseconds timeLimit);
void resetSearchState();
