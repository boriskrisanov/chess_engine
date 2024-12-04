#pragma once

#include "Board.hpp"

struct MCTSResult
{
    double whiteWinProbability;
    double blackWinProbability;
    double drawProbability;
};

MCTSResult mcts(Board board, uint64_t iterations);