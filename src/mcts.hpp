#pragma once

#include "Board.hpp"

enum class GameResult
{
    WHITE_WON,
    BLACK_WON,
    DRAW
};

struct MctsNodeStats
{
    uint32_t whiteWins = 0;
    uint32_t blackWins = 0;
    uint32_t draws = 0;

    uint64_t visits() const
    {
        return whiteWins + blackWins + draws;
    }

    void update(GameResult gameResult);
};

void startMcts(Board board);
void stopMcts();