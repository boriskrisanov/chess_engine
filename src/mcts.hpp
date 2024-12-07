#pragma once

#include "Board.hpp"

struct MCTSResult
{
    double winProbability;
    double lossProbability;
    double drawProbability;
};

MCTSResult mcts(Board board, uint64_t iterations);