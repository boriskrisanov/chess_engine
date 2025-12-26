#include "mcts.hpp"
#include "Move.hpp"
#include "Piece.hpp"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <unordered_map>

using node_hashmap_t = std::unordered_map<uint64_t, MctsNodeStats>;

std::random_device randomDevice;
std::mt19937 rng(randomDevice());

node_hashmap_t nodes;
bool shouldStopMcts = false;

void MctsNodeStats::update(GameResult gameResult)
{
    if (gameResult == GameResult::WHITE_WON)
    {
        whiteWins++;
    }
    else if (gameResult == GameResult::BLACK_WON)
    {
        blackWins++;
    }
    else
    {
        draws++;
    }
}

GameResult rollout(Board board)
{
    while (!board.getLegalMoves().empty() && !board.isDraw())
    {
        MoveList moves = board.getLegalMoves();
        std::uniform_int_distribution uniformIntDistribution{0, static_cast<int>(moves.size() - 1)};
        // Board is copied so no need to undo moves
        const int index = uniformIntDistribution(rng);
        board.makeMove(moves[index]);
    }
    if (board.isCheckmate(PieceColor::WHITE))
    {
        return GameResult::BLACK_WON;
    }
    if (board.isCheckmate(PieceColor::BLACK))
    {
        return GameResult::WHITE_WON;
    }
    return GameResult::DRAW;
}

double calculateNodeScore(uint64_t node, uint64_t parent, PieceColor side)
{
    const auto parentNodeStats = nodes[parent];
    const auto currentNodeStats = nodes[node];
    const double winRatio = static_cast<double>(side == PieceColor::WHITE
                                                    ? currentNodeStats.whiteWins
                                                    : currentNodeStats.blackWins) /
        static_cast<double>(currentNodeStats.visits());
    const auto parentVisits = static_cast<double>(parentNodeStats.visits());
    const double explorationConstant = std::sqrt(2);
    const double explorationValue = std::sqrt(std::log(parentVisits) / static_cast<double>(currentNodeStats.visits()));
    return winRatio + explorationConstant * explorationValue;
}

GameResult mctsIteration(Board& board, PieceColor side)
{
    const uint64_t currentHash = board.getHash();

    // rollout() will handle the case when the game has ended by simply returning the game result with no iterations
    if (!nodes.contains(currentHash) || board.getLegalMoves().empty() || board.isDraw())
    {
        auto result = rollout(board);
        nodes[currentHash].update(result);
        return result;
    }

    // Select node

    Move selectedMove;
    double bestScore = 0;
    for (Move move : board.getLegalMoves())
    {
        board.makeMove(move);
        uint64_t newHash = board.getHash();

        if (!nodes.contains(newHash))
        {
            selectedMove = move;
            board.unmakeMove();
            break;
        }

        const double score = calculateNodeScore(newHash, currentHash, side);
        if (score > bestScore)
        {
            selectedMove = move;
            bestScore = score;
        }

        board.unmakeMove();
    }

    // Backpropagate

    board.makeMove(selectedMove);
    auto result = mctsIteration(board, side);
    nodes[currentHash].update(result);
    board.unmakeMove();
    return result;
}

void printMctsStats(Board board)
{
    // Find most visited node
    uint64_t mostVisitedNode;
    uint64_t maxVisits = 0;
    Move selectedMove;
    for (Move move : board.getLegalMoves())
    {
        board.makeMove(move);

        if (nodes[board.getHash()].visits() > maxVisits)
        {
            mostVisitedNode = board.getHash();
            maxVisits = nodes[board.getHash()].visits();
            selectedMove = move;
        }

        board.unmakeMove();
    }

    const auto visits = nodes[mostVisitedNode].visits();
    const auto wins = board.sideToMove == PieceColor::WHITE ? nodes[mostVisitedNode].whiteWins : nodes[mostVisitedNode].blackWins;
    const auto losses = board.sideToMove == PieceColor::WHITE ? nodes[mostVisitedNode].blackWins : nodes[mostVisitedNode].whiteWins;
    const auto draws = visits - wins - losses;

    std::cout << std::setprecision(8) << std::fixed;
    std::cout << "Selected move: " << static_cast<std::string>(selectedMove) << "\n";
    std::cout << "w = " << static_cast<double>(wins) / visits << "\n";
    std::cout << "d = " <<  static_cast<double>(draws) / visits << "\n";
    std::cout << "l = " <<  static_cast<double>(losses) / visits << std::endl;
}

void mcts(Board board)
{
    shouldStopMcts = false;
    auto hash = board.getHash();
    auto side = board.sideToMove;
    while (!shouldStopMcts)
    {
        mctsIteration(board, side);
        // side = oppositeColor(side);
        const uint64_t iterations = nodes[hash].visits();
        if (iterations % 1000 == 0) [[unlikely]]
        {
            std::cout << iterations << " =====\n";
            printMctsStats(board);
            std::cout << "=====\n";
        }
    }

    printMctsStats(board);
    nodes.clear();
}

std::thread mctsThread;

void startMcts(Board board)
{
    mctsThread = std::thread{mcts, board};
    mctsThread.detach();
}

void stopMcts()
{
    shouldStopMcts = true;
}
