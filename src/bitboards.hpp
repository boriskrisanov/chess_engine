#pragma once

#include <bit>
#include <cstdint>
#include <vector>
#include "Move.hpp"

typedef uint64_t Bitboard;

namespace bitboards
{
    constexpr Bitboard withSquare(Square square)
    {
        return static_cast<Bitboard>(1) << 63 - square;
    }

    /**
     * Returns the index of the most significant bit and removes it from the bitboard
     */
    inline Square popMSB(Bitboard& bitboard)
    {
        const Square index = std::countl_zero(bitboard);
        bitboard &= ~withSquare(index);
        return index;
    }

    /**
     * Returns the index of the most significant bit
     */
    inline Square getMSB(Bitboard bitboard)
    {
        return std::countl_zero(bitboard);
    }

    /**
     * Returns the index of the most significant bit
     */
    inline Square getLSB(Bitboard bitboard)
    {
        return 63 - std::countr_zero(bitboard);
    }

    std::vector<Square> squaresOf(Bitboard bitboard);
    constexpr Bitboard ALL_SQUARES = 0xFFFFFFFFFFFFFFFF;
    constexpr Bitboard RANK_1 = 0x00000000000000FF;
    constexpr Bitboard RANK_4 = 0x00000000FF000000;
    constexpr Bitboard RANK_5 = 0x000000FF00000000;
    constexpr Bitboard RANK_8 = 0xFF00000000000000;

    constexpr Bitboard FILE_A = 0x8080808080808080;
    constexpr Bitboard FILE_H = 0x0101010101010101;
}
