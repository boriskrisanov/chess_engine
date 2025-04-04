#pragma once

#include <array>
#include <string>
#include <vector>


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
Magics findMagics(size_t iterations, const std::vector<std::vector<uint64_t>> &blockerPositions);

void printMagics(Magics m, std::string pieceName);

void findRookMagics(size_t iterations);

void findBishopMagics(size_t iterations);