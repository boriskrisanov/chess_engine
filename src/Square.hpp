#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

typedef uint8_t Square;

namespace square
{

// TODO: Precompute array
constexpr uint8_t rank(Square square)
{
    return 8 - square / 8;
}

constexpr uint8_t file(Square square)
{
    return square % 8 + 1;
}

inline Square fromString(const std::string &s)
{
    if (s.length() != 2)
    {
        throw std::runtime_error{"Invalid square " + s};
    }

    const uint8_t file = tolower(s.at(0)) - 'a' + 1;
    const uint8_t rank = std::stoi(std::string{s.at(1)});

    if (file < 1 || file > 8 || rank < 1 || rank > 8)
    {
        throw std::runtime_error{"Invalid square " + s};
    }

    const uint8_t down = 8 * (8 - rank);
    const uint8_t left = file - 1;

    return down + left;
}

inline std::string toString(Square square)
{
    uint8_t rank = 8;
    char file = 'a';
    for (int i = 0; i < square; i++)
    {
        if (file == 'h')
        {
            rank--;
            file = 'a';
            continue;
        }
        file++;
    }
    return file + std::to_string(rank);
}
} // namespace square
