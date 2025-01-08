#pragma once

#include "Board.hpp"

struct MCTSResult
{
    double whiteWinProbability;
    double blackWinProbability;
    double drawProbability;
};

Move mcts(Board board, uint64_t iterations);
MCTSResult mctsEval(Board board, uint64_t mctsIterations, uint64_t totalIterations);