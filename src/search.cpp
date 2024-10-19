#include "search.hpp"

#include <algorithm>
#include <corecrt_startup.h>
#include <iostream>
#include <vector>

#include "eval.hpp"

using std::vector;

// +1 and -1 to avoid overflow when multiplying by -1
constexpr int POSITIVE_INFINITY = std::numeric_limits<int>::max() - 1;
constexpr int NEGATIVE_INFINITY = std::numeric_limits<int>::min() + 1;
constexpr size_t TRANSPOSITION_TABLE_SIZE_MB = 10;

DebugStats debugStats{};

enum class NodeKind
{
    EMPTY,
    UPPER_BOUND,
    LOWER_BOUND,
    EXACT
};

struct TT_Entry
{
    NodeKind kind = NodeKind::EMPTY;
    uint64_t hash = 0;
    uint8_t depth = 0;
    int eval = 0;

    TT_Entry(NodeKind kind, uint64_t hash, uint8_t depth, int eval)
        : kind(kind), hash(hash), depth(depth), eval(eval)
    {
    }

    TT_Entry() = default;
};

constexpr size_t TRANSPOSITION_TABLE_ENTRIES = TRANSPOSITION_TABLE_SIZE_MB * 1000 * 1000 / sizeof(TT_Entry);
vector<TT_Entry> transpositionTable{TRANSPOSITION_TABLE_ENTRIES};

size_t index(uint64_t hash)
{
    return hash % TRANSPOSITION_TABLE_ENTRIES;
}

const TT_Entry* getTransposition(uint64_t hash)
{
    const TT_Entry* value = &transpositionTable.at(index(hash));
    if (value->kind == NodeKind::EMPTY || value->hash != hash)
    {
        // Empty node or index collision
        return nullptr;
    }
    return value;
}

void storeTransposition(NodeKind kind, uint64_t hash, uint8_t depth, int eval)
{
    transpositionTable.at(index(hash)) = {kind, hash, depth, eval};
}

int moveScore(const Board& board, const Move& move)
{
    int score = 0;
    Piece piece = board[move.start()];
    // This won't be the case for en passant, but it's so rare that it's probably faster to do this without branching and possibly have a suboptimal move order with en passant moves (TODO: Benchmark)
    Piece capturedPiece = board[move.end()];

    // Capturing a high value piece with a low value piece is likely to be a good move
    if (!capturedPiece.isNone())
    {
        score += pieceValue(capturedPiece.kind);
    }

    if (move.isPromotion())
    {
        score += 500;
    }

    return score;
}

void orderMoves(Board& board, vector<Move>& moves)
{
    for (Move& move : moves)
    {
        move.score = moveScore(board, move);
    }
    std::sort(moves.begin(), moves.end(), [](const Move& m1, const Move& m2)
    {
        return m1.score > m2.score;
    });
}


// Alpha - lower bound, beta - upper bound
// Anything less than alpha is useless because there's already a better line available
// Beta is the worst possible score for the opponent, anything higher than beta will not be chosen by the opponent
int evaluate(Board& board, uint8_t depth, uint8_t ply, int alpha, int beta)
{
    const TT_Entry* ttEntry = getTransposition(board.getHash());
    if (ttEntry != nullptr && false)
    {
        if (ttEntry->depth >= depth)
        {
            const int ttEval = ttEntry->eval;
            if (ttEntry->kind == NodeKind::LOWER_BOUND && ttEval > beta)
            {
                return ttEval;
            }
            if (ttEntry->kind == NodeKind::UPPER_BOUND && ttEval <= alpha)
            {
                return ttEval;
            }
            if (ttEntry->kind == NodeKind::EXACT)
            {
                return ttEval;
            }
        }
    }

    if (depth == 0)
    {
        // TODO: Store in TT? (depends on eval function complexity)
        debugStats.positionsEvaluated++;
        return staticEval(board);
    }

    vector<Move> moves = board.getLegalMoves();
    orderMoves(board, moves);

    // Assume that no moves will exceed alpha
    NodeKind nodeKind = NodeKind::UPPER_BOUND;

    for (const Move move : moves)
    {
        board.makeMove(move);
        /*
        Swap alpha and beta because the maximising player is now the minimising player and vice versa.
        Both are negative because the values are from the perspective of the side to move, which will now be reversed,
        and a good position for the minimising player is bad for the maximising player and vice versa.
         */
        const int eval = -evaluate(board, depth - 1, ply + 1, -beta, -alpha);
        board.unmakeMove();
        if (eval >= beta)
        {
            // Beta cutoff - a move earlier in the tree was too good and won't be chosen by the opponent
            // (there is a move the opponent can play to avoid this position, so this move will never be played)
            // This is a lower bound on the true eval because we are exiting the search early and there may be other
            // moves we haven't searched which may be better.
            storeTransposition(NodeKind::LOWER_BOUND, board.getHash(), depth, beta);
            return beta;
        }
        if (eval > alpha)
        {
            // This move is better than what we had before, so we will search it fully
            nodeKind = NodeKind::EXACT;
            alpha = eval;
        }
        // If eval <= alpha, we don't need to consider this move because we already have a better or equally good move available
    }

    if (moves.empty())
    {
        // Don't store these in TT since they are easy to compute (TODO: Benchmark this)
        if (board.isDraw())
        {
            return 0;
        }
        if (board.isSideInCheck(board.sideToMove))
        {
            // Checkmates closer to the root are better, so they should have a lower score
            // Not doing this causes the engine to make draws and not play the best move, even if it knows that it
            // can be played.
            int mateEval = NEGATIVE_INFINITY + 255 - depth;
            return mateEval;
        }
    }

    storeTransposition(nodeKind, board.getHash(), depth, alpha);
    return alpha;
}

SearchResult bestMove(Board& board, uint8_t depth)
{
    debugStats = DebugStats{};
    vector<Move> moves = board.getLegalMoves();

    // TODO: This will crash if there are no legal moves (mate/stalemate)
    Move bestMove = moves.at(0);
    int bestEval = NEGATIVE_INFINITY;

    for (Move move : moves)
    {
        board.makeMove(move);
        int eval = -evaluate(board, depth - 1, 1, NEGATIVE_INFINITY, POSITIVE_INFINITY);
        if (eval >= bestEval)
        {
            bestMove = move;
            bestEval = eval;
        }
        board.unmakeMove();
    }

    return {board.sideToMove, bestMove, bestEval};
}
