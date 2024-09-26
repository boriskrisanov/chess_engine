#include "bitboards.hpp"

#include <bit>

namespace bitboards
{
    std::vector<Square> squaresOf(Bitboard bitboard)
    {
        std::vector<Square> squares;
        squares.reserve(std::popcount(bitboard));

        for (int i = 0; i < 64; i++) {
            if ((bitboard & 1) == 1) {
                squares.push_back(63 - i);
            }
            bitboard >>= 1;
        }

        return squares;
    }
}
