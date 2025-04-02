#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

#include "Board.hpp"
#include "Move.hpp"
#include "utils.hpp"

int passedTests = 0;
int failedTests = 0;

inline size_t perft(Board &board, uint8_t depth, bool rootNode = true)
{
    size_t positionsReached = 0;

    // If running perft(1), print the full move list for debugging purposes
    if (depth == 1 && !rootNode)
    {
        return board.getLegalMoves().size();
    }
    if (depth == 0)
    {
        return 1;
    }

    for (Move move : board.getLegalMoves())
    {
        board.makeMove(move);

        const size_t result = perft(board, depth - 1, false);
        positionsReached += result;

        if (rootNode)
        {
            std::cout << static_cast<std::string>(move) << ": " << result << "\n";
        }

        board.unmakeMove();
    }

    return positionsReached;
}

inline size_t runPerft(uint8_t depth, const std::string &fen, const std::string &moveSequence)
{
    Board board;
    board.loadFen(fen);

    if (!moveSequence.empty())
    {
        if (moveSequence.length() == 4)
        {
            board.makeMove(moveSequence);
        }
        else
        {
            for (const std::string &move : splitString(moveSequence, " "))
            {
                board.makeMove(move);
            }
        }
    }

    std::map<std::string, size_t> moveCounts;
    auto start = std::chrono::system_clock::now();
    size_t total = perft(board, depth);
    auto end = std::chrono::system_clock::now();
    std::cout << total << " positions reached in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "\n";

    return total;
}

inline void runPerft(uint8_t depth, const std::string &fen)
{
    runPerft(depth, fen, "");
}

inline void test(uint8_t depth, const std::string &fen, size_t expectedValue)
{
    Board board;
    board.loadFen(fen);
    size_t total = perft(board, depth);
    std::cout << "test " << fen << " ";
    if (total == expectedValue)
    {
        std::cout << "PASSED (" << total << ")";
        passedTests++;
    }
    else
    {
        std::cout << "FAILED (expected " << expectedValue << " actual " << total
                  << ")";
        failedTests++;
    }
    std::cout << "\n";
}

inline void runTests()
{
    passedTests = 0;
    failedTests = 0;
    // TODO: Multithreading
    test(6, STARTING_POSITION_FEN, 119060324);
    // Test positions
    test(5,
         "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0",
         193690690);
    test(6, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0", 11030083);
    test(5, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
         15833292);
    test(5, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
         89941194);
    test(5,
         "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
         "10",
         164075551);
    test(6, "2bqk3/prpppp2/2n4b/8/1P6/8/2PPPPPN/RNBQKBNr b Q - 0 15", 367853606);
    test(5,
         "r3k1nr/p1ppprpp/Q1n1b1BP/Pp1bP3/2qPrP1b/NP1p1pP1/P1P1P1pP/R1BQKBNR w "
         "KQkq - 0 1",
         71379963);
    test(6, "8/k1p5/8/KP5r/8/8/6p1/4R2N w - - 0 1", 64081091);
    test(5, "q6r/1k6/8/8/8/8/1K6/Q6R w - - 0 1", 16871195);
    test(7, "k7/pppppppp/8/8/8/8/PPPPPPPP/K7 w - - 0 1", 303041957);

    // Positions from real games

    // https://lichess.org/QR5UbqUY#16
    test(5, "r1bqk2r/ppp2ppp/2n1pn2/8/QbBP4/2N2N2/PP3PPP/R1B2RK1 w kq - 4 9",
         108181315);

    // https://lichess.org/INY3KINN#51
    test(6, "2rr2k1/5np1/1pp1pn1p/p4p2/P1PP4/3NP1P1/5PP1/2RRB1K1 b - - 0 26",
         406683732);

    // https://lichess.org/INY3KINN#115
    test(6, "6k1/6p1/7p/2N3P1/PR6/5PK1/r5P1/6n1 b - - 2 58", 85338565);

    // https://lichess.org/751DRMPG#29
    test(5, "r2q1rk1/4bppp/1p2pn2/3pP3/2p2B2/4P2P/1PPNQPP1/R4RK1 b - - 0 15",
         63507755);

    // https://lichess.org/751DRMPG#89
    test(6, "3Q4/5k1N/4q1p1/3pB3/8/5P2/r5P1/6K1 b - - 4 45", 509977948);

    // https://lichess.org/I5iGXY21#108
    test(7, "8/8/8/p6p/P3R1r1/2k5/4K3/8 w - - 1 55", 234461080);

    // Played in engine test game
    test(5, "r2q1rk1/ppp2p1p/1bn5/7R/1P1p2b1/N1P5/P4QP1/R1B1KBN1 b Q - 0 19",
         101255241);

    std::cout << "Tests run: " << (passedTests + failedTests)
              << ". Passed: " << passedTests
              << ". Failed: " << failedTests
              << "\n";
    if (failedTests == 0)
    {
        std::cout << "All tests passed\n";
    }
    else
    {
        std::exit(1);
    }
}
