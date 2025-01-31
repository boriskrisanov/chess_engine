#pragma once

#include <array>
#include "Move.hpp"

struct MoveList
{
    void push_back(Move move)
    {
        moves[count++] = move;
    }

    void emplace_back(Square start, Square end, MoveFlag moveFlag)
    {
        moves[count++] = Move{start, end, moveFlag};
    }

    Move operator[](size_t i) const
    {
        return moves[i];
    }

    bool empty() const
    {
        return count == 0;
    }

    Move* begin()
    {
        return &moves[0];
    }

    Move* end()
    {
        return &moves[count];
    }

    size_t size() const
    {
        return count;
    }

private:
    std::array<Move, 218> moves;
    uint8_t count = 0;
};