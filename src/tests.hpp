#pragma once

#include <cstdlib>
#include <string>

size_t runPerft(uint8_t depth, const std::string &fen, const std::string &moveSequence);

inline void runPerft(uint8_t depth, const std::string &fen)
{
    runPerft(depth, fen, "");
}

void test(uint8_t depth, const std::string &fen, size_t expectedValue);

void runTests();