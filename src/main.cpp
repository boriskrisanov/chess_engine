#include "Board.hpp"
#include "eval.hpp"
#include "magic_searcher.hpp"
#include "search.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>
#include "mcts.hpp"

#include <thread>

using std::cin, std::cout, std::string;
using std::chrono::system_clock;

int main()
{
    Board board;
    board.loadFen(STARTING_POSITION_FEN);

    while (true)
    {
        string command;
        cin >> command;
        if (command == "position")
        {
            string mode;
            std::cin >> mode;
            string remainingCommand;
            if (mode == "fen")
            {
                string fen;
                std::getline(cin, fen);
                // Remove leading space
                fen = fen.erase(0, 1);
                if (fen.contains(" moves "))
                {
                    remainingCommand = splitString(fen, " moves ")[1];
                    fen = splitString(fen, " moves ")[0];
                }
                try
                {
                    board.loadFen(fen);
                }
                catch (std::invalid_argument &e)
                {
                    std::cout << e.what() << "\n";
                }
            }
            else if (mode == "startpos")
            {
                board.loadFen(STARTING_POSITION_FEN);
                std::getline(cin, remainingCommand);
            }
            if (!remainingCommand.empty())
            {
                remainingCommand = splitString(remainingCommand, " moves ")[1];
                // Move list
                for (const std::string &move : splitString(remainingCommand, " "))
                {
                    if (move.empty() || move.contains(" "))
                    {
                        continue;
                    }
                    board.makeMove(Move{board, move});
                }
            }
        }
        else if (command == "go")
        {
            string mode;
            cin >> mode;
            if (mode == "depth")
            {
                resetSearchState();
                int depth;
                cin >> depth;
                if (depth < 0)
                {
                    cout << "Invalid depth";
                    continue;
                }
                auto start = system_clock::now();
                SearchResult searchResult = bestMove(board, depth);
                auto end = system_clock::now();
                cout << "bestmove " << static_cast<string>(searchResult.bestMove) << "\n";
                cout << "eval " << searchResult.standardEval() << "\n";
                cout << "time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start) << "\n";
                cout << "positions evaluated: " << debugStats.positionsEvaluated << "\n";
                cout << "TT writes: " << debugStats.ttWrites << "\n";
                cout << "TT hits: " << debugStats.ttHits << "\n";
            }
            else if (mode == "time") // Not a standard UCI command
            {
                resetSearchState();
                uint32_t timeLimitMilliseconds;
                cin >> timeLimitMilliseconds;
                SearchResult searchResult = timeLimitedSearch(board, std::chrono::milliseconds{timeLimitMilliseconds});
                cout << "bestmove " << static_cast<string>(searchResult.bestMove) << "\n";
                cout << "eval " << searchResult.standardEval() << "\n";
                cout << "positions evaluated: " << debugStats.positionsEvaluated << "\n";
            }
            else if (mode == "perft")
            {
                int depth;
                cin >> depth;
                runPerft(depth, board.getFen());
            }
        }
        else if (command == "d")
        {
            cout << board.toString() << "\n";
            cout << "FEN: " << board.getFen() << "\n";
            cout << "Hash: " << board.getHash() << "\n";
            cout << "--- Evaluation ---" << "\n";
            printDebugEval(board);
        }
        else if (command == "test")
        {
            runTests();
        }
        else if (command == "magics")
        {
            int iterations;
            std::cin >> iterations;
            findRookMagics(iterations);
            findBishopMagics(iterations);
        }
        else if (command == "quit")
        {
            break;
        }
        else if (command == "mcts")
        {
            startMcts(board);
        }
        else if (command == "stop")
        {
            stopMcts = true;
        }
        else
        {
            cout << "Invalid command" << "\n";
        }
    }
}