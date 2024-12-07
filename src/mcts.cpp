#include "mcts.hpp"
#include <cstdint>
#include <iostream>
#include <random>
#include <unordered_map>

#include "search.hpp"

struct Node
{
    uint64_t wins = 0;
    uint64_t draws = 0;
    uint64_t losses = 0;
    uint64_t visits = 0;
};

enum class GameResult
{
    NONE,
    WIN,
    DRAW,
    LOSS
};

GameResult getGameResult(Board& board, PieceColor side)
{
    using enum GameResult;
    if (board.isDraw())
    {
        return DRAW;
    }
    if (board.isCheckmate(side))
    {
        std::cout << "Loss\n";
        return LOSS;
    }
    if (board.isCheckmate(oppositeColor(side)))
    {
        return WIN;
    }
    return NONE;
}

GameResult MCTSRollout(Board board, std::mt19937& rng, PieceColor side)
{
    GameResult gameResult = getGameResult(board, side);
    while (gameResult == GameResult::NONE)
    {
        const std::vector<Move> moves = board.getLegalMoves();
        std::uniform_int_distribution<size_t> uniformIntDistribution{0, moves.size() - 1};
        Move move = moves[uniformIntDistribution(rng)];
        board.makeMove(move);
        gameResult = getGameResult(board, side);
    }
    return gameResult;
}

double UCB_Score(const Node& parent, const Node& child)
{
    using std::sqrt, std::log;
    const double winRatio = static_cast<double>(child.wins) / static_cast<double>(child.visits);
    const double explorationConstant = sqrt(2);
    const double explorationParameter = sqrt(log(static_cast<double>(parent.visits)) / static_cast<double>(child.visits));
    return winRatio + explorationConstant * explorationParameter;
}

void MCTSIteration(std::unordered_map<uint64_t, Node>& nodes, Board board, std::mt19937& rng,
                   std::vector<uint64_t>& visitedNodes, const PieceColor side)
{
    visitedNodes.clear();
    uint64_t currentNode = board.getHash();
    const auto moves = board.getLegalMoves();
    GameResult gameResult = getGameResult(board, side);
    while (gameResult == GameResult::NONE)
    {
        // Selection
        // This is ok because there will always be at least one legal move (otherwise the loop wouldn't run)
        Move selectedMove = moves[0];
        double bestScore = 0;
        for (Move move : moves)
        {
            board.makeMove(move);
            uint64_t newNode = board.getHash();
            board.unmakeMove();

            if (!nodes.contains(newNode))
            {
                // Node has never been visited
                nodes[newNode] = Node{};
                const GameResult rolloutResult = MCTSRollout(board, rng, side);
                visitedNodes.push_back(currentNode);
                visitedNodes.push_back(newNode);
                for (uint64_t node : visitedNodes)
                {
                    nodes[node].visits++;
                    switch (rolloutResult)
                    {
                    case GameResult::WIN:
                        nodes[node].wins++;
                        return;
                    case GameResult::LOSS:
                        nodes[node].losses++;
                        return;
                    case GameResult::DRAW:
                        nodes[node].draws++;
                        return;
                        // ReSharper disable once CppDFAUnreachableCode
                    default: return;
                    }
                }
            }

            // UCB
            const double score = UCB_Score(nodes[board.getHash()], nodes[newNode]);
            if (score > bestScore)
            {
                bestScore = score;
                selectedMove = move;
            }
        }

        visitedNodes.push_back(currentNode);
        board.makeMove(selectedMove);
        currentNode = board.getHash();
        gameResult = getGameResult(board, side);
    }
    visitedNodes.push_back(currentNode);

    for (uint64_t node : visitedNodes)
    {
        nodes[node].visits++;
        if (gameResult == GameResult::WIN)
        {
            nodes[node].wins++;
        }
        else if (gameResult == GameResult::LOSS)
        {
            nodes[node].losses++;
        }
        else
        {
            nodes[node].draws++;
        }
    }
}

MCTSResult mcts(Board board, uint64_t iterations)
{
    std::unordered_map<uint64_t, Node> nodes;
    nodes[board.getHash()] = Node{};
    std::random_device randomDevice;
    std::mt19937 rng{randomDevice()};

    std::vector<uint64_t> visitedNodes{};
    const auto side = board.sideToMove;
    for (int i = 0; i < iterations; i++)
    {
        MCTSIteration(nodes, board, rng, visitedNodes, side);
    }

    uint64_t maxVisits = 0;
    // TODO: This will crash if there are no legal moves
    Move bestMove = board.getLegalMoves()[0];
    for (Move move : board.getLegalMoves())
    {
        board.makeMove(move);
        const uint64_t hash = board.getHash();
        board.unmakeMove();

        if (nodes[hash].visits > maxVisits)
        {
            maxVisits = nodes[hash].visits;
            bestMove = move;
        }
    }
    board.makeMove(bestMove);

    uint64_t wins = nodes[board.getHash()].wins;
    uint64_t losses = nodes[board.getHash()].losses;
    uint64_t draws = nodes[board.getHash()].draws;
    const double visits = static_cast<double>(nodes[board.getHash()].visits);

    return {static_cast<double>(wins) / visits, static_cast<double>(losses) / visits, static_cast<double>(draws) / visits};
}
