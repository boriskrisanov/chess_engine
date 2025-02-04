#pragma once

#include <random>

// Ported from Java version, can probably be improved (TODO)
class MagicFinder
{
public:
    /**
     * For each square, find a unique magic and shift value that allows for a unique mapping between the 64-bit bitboard
     * and a smaller value, which can be used as an array index in an array of attacking squares. The aim is to maximise
     * the shift in order to minimise the range of indexes and hence the array size.
     */
    void findMagics()
    {

    }
private:
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> uniformIntDistribution{0,std::numeric_limits<uint64_t>::max()};
};