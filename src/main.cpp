#include "tests.hpp"
#include <iostream>

#include "search.hpp"

using std::cin, std::cout, std::string;
using std::chrono::system_clock;

int main()
{
    Board board;
    board.loadFen(STARTING_POSITION_FEN);
    board.loadFen("r3kr2/pppqb1pp/2np1n2/4p2b/P1B1P3/2N1BN1P/1PP2PP1/R2Q1RK1 w q - 1 12");
    while (true)
    {
        string command;
        cin >> command;
        if (command == "position")
        {
            string mode;
            std::cin >> mode;
            if (mode == "fen")
            {
                string fen;
                std::getline(cin, fen);
                // Remove leading space
                fen = fen.erase(0, 1);
                board.loadFen(fen);
            }
            else if (mode == "startpos")
            {
                board.loadFen(STARTING_POSITION_FEN);
            }
        }
        else if (command == "go")
        {
            string mode;
            cin >> mode;
            if (mode == "depth")
            {
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
            }
        }
        else if (command == "d")
        {
            cout << board.toString() << "\n";
        }
        else if (command == "quit")
        {
            break;
        }
        else
        {
            cout << "Invalid command" << "\n";
        }
    }
}
