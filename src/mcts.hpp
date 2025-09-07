#pragma once

#include "Board.hpp"

#include <iomanip>
#include <random>
#include <thread>
#include <unordered_map>
#include <iostream>

static std::random_device randomDevice;
static std::mt19937 rng(randomDevice());

struct MctsNodeStats
{
    uint32_t whiteWins = 0;
    uint32_t blackWins = 0;
    uint32_t draws = 0;
    bool isLeaf = true;

    uint64_t visits() const
    {
        return whiteWins + blackWins + draws;
    }
};

enum class GameResult
{
    WHITE_WON,
    BLACK_WON,
    DRAW
};

using node_hashmap_t = std::unordered_map<uint64_t, MctsNodeStats>;
static node_hashmap_t wNodes;
static node_hashmap_t bNodes;


static std::vector<uint64_t> visitedNodes; // Cleared on every iteration, defined here for performance

inline GameResult rollout(Board board)
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

inline void updateVisitedNodeStats(GameResult gameResult, node_hashmap_t &nodes)
{
    for (uint64_t hash : visitedNodes)
    {
        if (gameResult == GameResult::WHITE_WON)
        {
            nodes[hash].whiteWins++;
        }
        else if (gameResult == GameResult::BLACK_WON)
        {
            nodes[hash].blackWins++;
        }
        else if (gameResult == GameResult::DRAW)
        {
            nodes[hash].draws++;
        }
    }
}

inline void mctsIteration(Board board, PieceColor side)
{
    // NOTE: operator[] will create the node if it doesn't exist in the hashmap

    visitedNodes.clear();
    visitedNodes.push_back(board.getHash());

    node_hashmap_t& nodes = side == PieceColor::WHITE ? wNodes : bNodes;

    MoveList legalMoves = board.getLegalMoves();

    if (!legalMoves.empty() && !board.isDraw())
    {
        nodes[board.getHash()].isLeaf = false;
    }

    while (!legalMoves.empty() && !board.isDraw())
    {
        // Select next move
        double bestScore = 0;
        Move selectedMove;
        for (Move move : legalMoves)
        {
            board.makeMove(move);
            const auto newHash = board.getHash();
            const MctsNodeStats &newNode = nodes[newHash];

            if (newNode.visits() == 0)
            {
                const auto result = rollout(board);
                visitedNodes.push_back(newHash);
                updateVisitedNodeStats(result, nodes);
                return;
            }
            // Calculate score
            const double winRatio = static_cast<double>(side == PieceColor::WHITE ? newNode.whiteWins : newNode.blackWins) / static_cast<double>(newNode.visits());
            const auto parentNodeStats = nodes[visitedNodes[visitedNodes.size() - 1]];
            const auto parentVisits = static_cast<double>(parentNodeStats.visits());
            const double explorationConstant = std::sqrt(2);
            const double explorationValue = std::sqrt(std::log(parentVisits) / static_cast<double>(newNode.visits()));
            const double score = winRatio + explorationConstant * explorationValue;

            if (score > bestScore)
            {
                bestScore = score;
                selectedMove = move;
            }

            board.unmakeMove();
        }

        board.makeMove(selectedMove);
        visitedNodes.push_back(board.getHash());

        legalMoves = board.getLegalMoves();
    }

    for (const uint64_t hash : visitedNodes)
    {
        if (board.isCheckmate(PieceColor::WHITE))
        {
            updateVisitedNodeStats(GameResult::BLACK_WON, nodes);
        }
        else if (board.isCheckmate(PieceColor::BLACK))
        {
            updateVisitedNodeStats(GameResult::WHITE_WON, nodes);
        }
        else if (board.isDraw())
        {
            updateVisitedNodeStats(GameResult::DRAW, nodes);
        }
    }
}

// TODO: Improve threading code (this is temporary, just testing for now)

static bool stopMcts = false;

inline double calculateConfidence(const node_hashmap_t& nodes)
{
    // TODO: Improve
    double meanVisitCount = 0;
    double visitCountSquareSum = 0;

    std::vector<MctsNodeStats> leafNodes;

    for (const auto[hash, node] : nodes)
    {
        if (node.isLeaf)
        {
            leafNodes.push_back(node);
        }
    }

    for (const MctsNodeStats& node : leafNodes)
    {
        meanVisitCount += node.visits();
    }

    meanVisitCount /= leafNodes.size();

    for (const MctsNodeStats& node : leafNodes)
    {
        visitCountSquareSum += std::pow(node.visits() - meanVisitCount, 2);
    }

    const double stddev = std::sqrt(visitCountSquareSum / leafNodes.size());

    return stddev;
}

inline void printMctsStats(uint64_t hash)
{
    // TODO: Clean up code
    double totalVisits = wNodes[hash].visits() + bNodes[hash].visits();

    // Stats from white's perspective
    double w1 = static_cast<double>(wNodes[hash].whiteWins) / totalVisits;
    double b1 = static_cast<double>(wNodes[hash].blackWins) / totalVisits;
    double d1 = static_cast<double>(wNodes[hash].draws) / totalVisits;



    // Stats from black's perspective
    double w2 = static_cast<double>(bNodes[hash].whiteWins + bNodes[hash].whiteWins) / totalVisits;
    double b2 = static_cast<double>(bNodes[hash].blackWins + bNodes[hash].blackWins) / totalVisits;
    double d2 = static_cast<double>(bNodes[hash].draws + bNodes[hash].draws) / totalVisits;

    std::cout << std::setprecision(8) << std::fixed;
    std::cout << "W: w = " << w1 << ", b = " << b1 << ", d = " << d1 << ", confidence = " << calculateConfidence(wNodes) << "\n";
    std::cout << "B: w = " << w2 << ", b = " << b2 << ", d = " << d2 << ", confidence = " << calculateConfidence(bNodes) << "\n";
    std::cout << std::endl;
}

inline void mcts(Board board)
{
    stopMcts = false;
    auto hash = board.getHash();
    wNodes.clear();
    bNodes.clear();
    auto side = PieceColor::WHITE;
    while (!stopMcts)
    {
        mctsIteration(board, side);
        side = oppositeColor(side);
        const uint64_t iterations = wNodes[hash].visits() + bNodes[hash].visits();
        if (iterations % 1000 == 0) [[unlikely]]
        {
            std::cout << iterations << " =====";
            printMctsStats(hash);
            std::cout << "=====\n";
        }
    }

    printMctsStats(hash);

    visitedNodes.clear();
    wNodes.clear();
    bNodes.clear();
}

inline std::thread mctsThread;

inline void startMcts(Board board)
{
    mctsThread = std::thread{mcts, board};
    mctsThread.detach();
}