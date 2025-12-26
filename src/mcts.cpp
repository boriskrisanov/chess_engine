#include "mcts.hpp"

#include <complex>
#include <iomanip>
#include <thread>
#include <unordered_map>
#include <iostream>

using node_hashmap_t = std::unordered_map<uint64_t, MctsNodeStats>;

std::random_device randomDevice;
std::mt19937 rng(randomDevice());

node_hashmap_t nodes;

std::vector<uint64_t> visitedNodes; // Cleared on every iteration, defined here for performance

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

void updateVisitedNodeStats(GameResult gameResult, node_hashmap_t &nodes)
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


void mctsIteration(Board board)
{
    // NOTE: operator[] will create the node if it doesn't exist in the hashmap

    visitedNodes.clear();
    visitedNodes.push_back(board.getHash());
    const PieceColor side = board.sideToMove;

    MoveList legalMoves = board.getLegalMoves();

    if (!legalMoves.empty() && !board.isDraw())
    {
        nodes[board.getHash()].isLeaf = false;
    }

    while (!legalMoves.empty() && !board.isDraw())
    {
        // Select next move
        double bestScore = 0;
        Move selectedMove = legalMoves[0];
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

bool shouldStopMcts = false;

void printMctsStats(uint64_t hash)
{
    std::vector<MctsNodeStats> leaves;
    uint64_t totalIterations = nodes[hash].visits();
    uint64_t totalWhiteWins = nodes[hash].whiteWins;
    uint64_t totalBlackWins = nodes[hash].blackWins;
    uint64_t totalDraws = nodes[hash].draws;
    double w = 0;
    double b = 0;
    double d = 0;
    for (const auto&[node, stats] : nodes)
    {
        if (stats.isLeaf)
        {
            const double weight = static_cast<double>(stats.visits()) / totalIterations;
            w += (static_cast<double>(stats.whiteWins) / totalWhiteWins) * weight;
            b += (static_cast<double>(stats.blackWins) / totalBlackWins) * weight;
            d += (static_cast<double>(stats.draws) / totalDraws) * weight;
        }
    }

    // Softmax
    auto expSum = std::exp(w) + std::exp(b) + std::exp(d);

    std::cout << std::setprecision(8) << std::fixed;
    std::cout << "w = " << std::exp(w) / expSum << "\n";
    std::cout << "b = " << std::exp(b) / expSum << "\n";
    std::cout << "d = " << std::exp(d) / expSum << std::endl;
}

void mcts(Board board)
{
    shouldStopMcts = false;
    auto hash = board.getHash();
    nodes.clear();
    auto side = PieceColor::WHITE;
    while (!shouldStopMcts)
    {
        mctsIteration(board);
        // side = oppositeColor(side);
        const uint64_t iterations = nodes[hash].visits();
        if (iterations % 1000 == 0) [[unlikely]]
        {
            std::cout << iterations << " =====\n";
            printMctsStats(hash);
            std::cout << "=====\n";
        }
    }

    printMctsStats(hash);

    visitedNodes.clear();
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