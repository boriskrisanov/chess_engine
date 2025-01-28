#pragma once

#include <bit>
#include <cstdint>
#include <vector>
#include "Move.hpp"
#include <limits>

typedef uint64_t Bitboard;

namespace bitboards
{
    constexpr Bitboard withSquare(Square square)
    {
        return static_cast<Bitboard>(1) << (63 - square);
    }

    inline Square popMSB(Bitboard& bitboard)
    {
        Square index = std::countl_zero(bitboard);
        bitboard &= ~withSquare(index);
        return index;
    }

    std::vector<Square> squaresOf(Bitboard bitboard);
    constexpr Bitboard ALL_SQUARES = std::numeric_limits<Bitboard>::max();
}
