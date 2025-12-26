#pragma once

#include "Board.hpp"

#include <random>

struct MctsNodeStats
{
    uint32_t whiteWins = 0;
    uint32_t blackWins = 0;
    uint32_t draws = 0;
    bool isLeaf = true;

    uint64_t visits() const
    {
        return whiteWins + blackWins + draws;
    }
};

enum class GameResult
{
    WHITE_WON,
    BLACK_WON,
    DRAW
};

void mctsIteration(Board board);
void startMcts(Board board);
void stopMcts();