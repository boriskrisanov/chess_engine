#pragma once

#include "Board.hpp"

#include <unordered_map>

struct MctsNodeStats
{
    size_t whiteWins = 0;
    size_t blackWins = 0;
    size_t draws = 0;

    size_t visits() const
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

static std::unordered_map<uint64_t, MctsNodeStats> nodes;

inline GameResult mctsIteration(Board &board)
{
    // operator[] will create the node if it doesn't exist
    // Create node in hashmap if it doesn't already exist
    const auto hash = board.getHash();
    const auto currentNodeSide = board.sideToMove;
    // if (!nodes.contains(hash))
    // {
    //     nodes[hash] = MctsNodeStats{};
    // }

    // Return and update node stats if the game is over
    if (board.isCheckmate(PieceColor::WHITE))
    {
        nodes[hash].blackWins++;
        return GameResult::WHITE_WON;
    }
    if (board.isCheckmate(PieceColor::BLACK))
    {
        nodes[hash].whiteWins++;

        return GameResult::BLACK_WON;
    }
    if (board.isDraw())
    {
        nodes[hash].draws++;
        return GameResult::DRAW;
    }

    // Select next node
    MoveList moves = board.getLegalMoves();
    double bestScore = 0;
    Move selectedMove;
    for (Move move : moves)
    {
        board.makeMove(move);
        const auto newHash = board.getHash();
        const MctsNodeStats &newNode = nodes[newHash];

        // Calculate score
        if (newNode.visits() == 0)
        {
            // Win ratio will be infinite so score will be infinite
            selectedMove = move;
            board.unmakeMove();
            break;
        }
        const double winRatio = static_cast<double>(board.sideToMove == PieceColor::WHITE ? newNode.whiteWins : newNode.blackWins) / static_cast<double>(newNode.visits());
        const auto parentVisits = static_cast<double>(nodes[hash].visits());
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

    // No need to undo move because board is copied for each iteration
    board.makeMove(selectedMove);

    auto result = mctsIteration(board);

    if (result == GameResult::WHITE_WON)
    {
        nodes[hash].whiteWins++;
    }
    else if (result == GameResult::BLACK_WON)
    {
        nodes[hash].blackWins++;
    }
    else if (result == GameResult::DRAW)
    {
        nodes[hash].draws++;
    }

    return result;
}

inline void mcts(Board board, size_t iterations)
{
    auto hash = board.getHash();
    nodes.clear();
    for (int i = 0; i < iterations; i++)
    {
        Board boardCopy = board;
        mctsIteration(boardCopy);
    }

    auto stats = nodes[hash];
    double w = static_cast<double>(stats.whiteWins) / static_cast<double>(stats.visits());
    double b = static_cast<double>(stats.blackWins) / static_cast<double>(stats.visits());
    double d = static_cast<double>(stats.draws) / static_cast<double>(stats.visits());

    std::cout << "w = " << w << ", b = " << b << ", d = " << d << "\n";
}