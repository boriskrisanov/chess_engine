#include "bitboards.hpp"
#include <bit>

namespace bitboards
{
    Square popMSB(Bitboard& bitboard)
    {
        const Square index = std::countl_zero(bitboard);
        bitboard &= ~withSquare(index);
        return index;
    }

    Square getMSB(Bitboard bitboard)
    {
        return std::countl_zero(bitboard);
    }

    Square getLSB(Bitboard bitboard)
    {
        return 63 - std::countr_zero(bitboard);
    }

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
