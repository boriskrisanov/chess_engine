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

node_hashmap_t whiteNodes;
node_hashmap_t blackNodes;

node_hashmap_t& getNodes(PieceColor color)
{
    return color == PieceColor::WHITE ? whiteNodes : blackNodes;
}

int randint(int min, int max)
{
    std::uniform_int_distribution uniformIntDistribution{min, max};
    return uniformIntDistribution(rng);
}

Move randomMove(const MoveList& moves)
{
    return moves[randint(0, moves.size() - 1)];
}

struct MoveProbability
{
    Move move;
    double probability;

    friend bool operator<(const MoveProbability& lhs, const MoveProbability& rhs)
    {
        return lhs.probability < rhs.probability;
    }

    friend bool operator<=(const MoveProbability& lhs, const MoveProbability& rhs)
    {
        return !(rhs < lhs);
    }

    friend bool operator>(const MoveProbability& lhs, const MoveProbability& rhs)
    {
        return rhs < lhs;
    }

    friend bool operator>=(const MoveProbability& lhs, const MoveProbability& rhs)
    {
        return !(lhs < rhs);
    }

    friend bool operator==(const MoveProbability& lhs, const MoveProbability& rhs)
    {
        return lhs.probability == rhs.probability;
    }
};

// TODO: Make MoveList generic
// Index for move and its probability should be the same (TODO: Bad design but this is just a proof of concept)
Move randomMoveFromDistribution(std::vector<MoveProbability> probabilities)
{
    // Shuffle first to make selection between identical probabilities random
    std::ranges::shuffle(probabilities, rng);
    std::ranges::sort(probabilities);
    std::uniform_real_distribution<> distribution{0, 1};
    double sample = distribution(rng);
    for (int i = 0; i < probabilities.size(); i++)
    {
        if (sample < probabilities[i].probability)
        {
            return probabilities[i].move;
        }
    }
    return probabilities[probabilities.size() - 1].move;
}

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
    MoveList moves = board.getLegalMoves();
    while (!moves.empty() && !board.isDraw())
    {
        board.makeMove(randomMove(moves));
        moves = board.getLegalMoves();
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

double calculateNodeScore(uint64_t node, uint64_t parent, const PieceColor side)
{
    auto& nodes = getNodes(side);
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

GameResult mctsIteration(Board& board, const PieceColor side)
{
    const uint64_t currentHash = board.getHash();
    auto& nodes = getNodes(side);

    MoveList moves = board.getLegalMoves();
    // rollout() will handle the case when the game has ended by simply returning the game result with no iterations
    if (!nodes.contains(currentHash) || moves.empty() || board.isDraw())
    {
        auto result = rollout(board);
        nodes[currentHash].update(result); // TODO: Problem might be here?
        return result;
    }

    // Select node (at this point we know that there is at least one legal move)
    double bestScore = 0;
    Move selectedMove = moves[0];
    if (board.sideToMove == side)
    {
        // Select with UCT
        for (const Move move : moves)
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
    }
    else
    {
        // Select with weighted random
        // Compute probabilities
        auto adversaryNodes = getNodes(oppositeColor(side));
        std::vector<MoveProbability> moveProbabilities;
        const double totalVisitCount = adversaryNodes[board.getHash()].visits();
        for (Move move : board.getLegalMoves())
        {
            board.makeMove(move);
            const double visitCount = adversaryNodes[board.getHash()].visits();
            moveProbabilities.push_back({move, visitCount / totalVisitCount});
            board.unmakeMove();
        }
        selectedMove = randomMoveFromDistribution(moveProbabilities);
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
    auto& nodes = getNodes(board.sideToMove);
    const auto rootHash = board.getHash();
    const Board initialBoard = board;

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

        std::cout << board.getHash() << " score " << move.getPgn(initialBoard) << ": " << calculateNodeScore(
            board.getHash(), rootHash, board.sideToMove) << "\n";


        board.unmakeMove();
    }

    const auto visits = nodes[mostVisitedNode].visits();
    const auto wins = board.sideToMove == PieceColor::WHITE
                          ? nodes[mostVisitedNode].whiteWins
                          : nodes[mostVisitedNode].blackWins;
    const auto losses = board.sideToMove == PieceColor::WHITE
                            ? nodes[mostVisitedNode].blackWins
                            : nodes[mostVisitedNode].whiteWins;
    const auto draws = visits - wins - losses;

    std::cout << std::setprecision(8) << std::fixed;
    std::cout << "Selected move: " << static_cast<std::string>(selectedMove) << "\n";
    std::cout << "w = " << static_cast<double>(wins) / visits << "\n";
    std::cout << "d = " << static_cast<double>(draws) / visits << "\n";
    std::cout << "l = " << static_cast<double>(losses) / visits << "\n";
    std::cout << "Stored positions: " << nodes.size() << std::endl;
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
        const uint64_t iterations = whiteNodes[hash].visits() + blackNodes[hash].visits();
        if (iterations % 1000 == 0) [[unlikely]]
        {
            std::cout << iterations << " =====\n";
            printMctsStats(board);
            std::cout << "=====\n";
        }
        side = oppositeColor(side);
    }

    printMctsStats(board);
    whiteNodes.clear();
    blackNodes.clear();
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
