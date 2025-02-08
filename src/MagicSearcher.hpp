#pragma once

#include <array>
#include <iostream>
#include <random>
#include <unordered_set>

#include "movegen.hpp"

static std::random_device randomDevice;
static std::mt19937_64 rng{randomDevice()};
static std::uniform_int_distribution<uint64_t> uniformIntDistribution{0, std::numeric_limits<uint64_t>::max()};

struct Magics
{
    std::array<uint64_t, 64> magics{};
    std::array<int, 64> shifts{};
};

/**
 * For each square, find a unique magic and shift value that allows for a unique mapping between the 64-bit bitboard
 * and a smaller value, which can be used as an array index in an array of attacking squares. The aim is to maximise
 * the shift in order to minimise the range of indexes and hence the array size.
 */
inline Magics findMagics(size_t iterations, const std::vector<std::vector<uint64_t>>& blockerPositions)
{
    Magics m{};
    std::unordered_set<uint64_t> usedKeys;
    for (int iter = 0; iter < iterations; iter++)
    {
        for (int i = 0; i < 64; i++)
        {
            usedKeys.clear();
            const uint64_t magic = uniformIntDistribution(rng);
            bool collision = false;
            // shifts[i] is the best current value, so add 1 to search for a larger shift
            const int newShift = m.shifts[i] + 1;
            for (uint64_t blockers : blockerPositions[i])
            {
                uint64_t key = (blockers * magic) >> newShift;
                if (usedKeys.contains(key))
                {
                    collision = true;
                    break;
                }
                usedKeys.insert(key);
            }
            if (!collision)
            {
                m.magics[i] = magic;
                m.shifts[i] = newShift;
            }
        }
    }
    return m;
}

inline void printMagics(Magics m, std::string pieceName)
{
    std::string magicsOutput;
    magicsOutput += "constexpr array<uint64_t, 64> " + pieceName + "_MAGICS{";
    for (int i = 0; i < 64; i++)
    {
        magicsOutput += std::format("0x{:x}", m.magics[i]);
        if (i != 63)
        {
            magicsOutput += ", ";
        }
    }
    magicsOutput += "};";
    std::string shiftsOutput;
    shiftsOutput += "constexpr array<uint64_t, 64> " + pieceName + "_SHIFTS{";
    for (int i = 0; i < 64; i++)
    {
        shiftsOutput += std::to_string(m.shifts[i]);
        if (i != 63)
        {
            shiftsOutput += ", ";
        }
    }
    shiftsOutput += "};";

    std::cout << magicsOutput << "\n";
    std::cout << shiftsOutput << "\n";

    int totalBits = 0;
    for (int i = 0; i < 64; i++)
    {
        totalBits += 64 - m.shifts[i];
    }
    std::cout << "\nTotal size: " << totalBits << " bits \n\n";
}

inline void findRookMagics(size_t iterations)
{
    std::vector<std::vector<uint64_t>> blockerPositions;
    for (int i = 0; i < 64; i++)
    {
        blockerPositions.push_back(movegen::possibleBlockerPositions(movegen::getRookBlockerMasks()[i]));
    }
    Magics m = findMagics(iterations, blockerPositions);
    printMagics(m, "ROOK");
}

inline void findBishopMagics(size_t iterations)
{
    std::vector<std::vector<uint64_t>> blockerPositions;
    for (int i = 0; i < 64; i++)
    {
        blockerPositions.push_back(movegen::possibleBlockerPositions(movegen::getBishopBlockerMasks()[i]));
    }
    Magics m = findMagics(iterations, blockerPositions);
    printMagics(m, "BISHOP");
}
