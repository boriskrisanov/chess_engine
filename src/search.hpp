#pragma once

#include "Move.hpp"
#include "Piece.hpp"
#include <chrono>


struct DebugStats
{
    uint64_t positionsEvaluated = 0;
    uint64_t ttWrites = 0;
    uint64_t ttHits = 0;
};

extern DebugStats debugStats;

struct SearchResult
{
    PieceColor sideToMove;
    Move bestMove;
    int eval;
    int depthSearched;
    DebugStats debugStats;

    SearchResult(PieceColor sideToMove, Move bestMove, int eval, int depthSearched, const DebugStats &debugStats)
        : sideToMove(sideToMove), bestMove(bestMove), eval(eval), depthSearched(depthSearched), debugStats(debugStats)
    {
    }

    double standardEval() const
    {
        // Divide by 100 to convert centipawns to pawns
        return static_cast<double>(sideToMove == PieceColor::BLACK ? eval * -1 : eval) / 100;
    }
};

void resizeTranspositionTable(size_t sizeMB);

SearchResult bestMove(Board &board, uint8_t depth);
SearchResult timeLimitedSearch(Board &board, std::chrono::milliseconds timeLimit);
void resetSearchState();
