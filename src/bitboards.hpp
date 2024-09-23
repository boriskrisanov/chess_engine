#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include "Move.hpp"

typedef uint64_t Bitboard;

namespace bitboards
{
    constexpr Bitboard withSquare(Square square)
    {
        return static_cast<Bitboard>(1) << (63 - square);
    }

    std::vector<Square> squaresOf(Bitboard bitboard);
}
