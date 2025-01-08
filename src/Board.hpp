#pragma once

#include <array>
#include <stack>
#include <string>

#include "bitboards.hpp"
#include "Piece.hpp"

const std::string STARTING_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

using PieceColor::WHITE;
using PieceColor::BLACK;

struct BoardState
{
    int8_t enPassantTargetSquare;
    Bitboard whiteAttackingSquares;
    Bitboard whitePawnAttackingSquares;
    Bitboard blackAttackingSquares;
    Bitboard blackPawnAttackingSquares;
    bool whiteCanShortCastle;
    bool whiteCanLongCastle;
    bool blackCanShortCastle;
    bool blackCanLongCastle;
    uint8_t halfMoveClock;
};

class Board
{
public:
    void loadFen(std::string fen);
    std::string getFen() const;
    void makeMove(Move move);
    void makeMove(std::string uciMove);
    void unmakeMove();
    std::vector<Move> getLegalMoves();
    std::vector<Move> getLegalCaptures();
    void getPseudoLegalMoves(std::vector<Move>& moves) const;
    bool isSideInCheckAfterMove(Move move, PieceColor side);
    bool isPseudoLegalMoveLegal(Move move);
    std::string toString() const;
    std::string uciMoveHistory() const;
    Bitboard getSlidingPieces(PieceColor side) const;
    bool isDraw();

    Bitboard whitePawns = 0;
    Bitboard whiteKnights = 0;
    Bitboard whiteBishops = 0;
    Bitboard whiteRooks = 0;
    Bitboard whiteQueens = 0;
    Bitboard whiteKing = 0;
    Bitboard blackPawns = 0;
    Bitboard blackKnights = 0;
    Bitboard blackBishops = 0;
    Bitboard blackRooks = 0;
    Bitboard blackQueens = 0;
    Bitboard blackKing = 0;

    PieceColor sideToMove = WHITE;
    Square whiteKingPosition = 0;
    Square blackKingPosition = 0;

    operator std::string() const
    {
        return toString();
    }

    Bitboard getPieces(PieceColor color) const
    {
        return color == WHITE
                   ? whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing
                   : blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing;
    }

    Bitboard getPieces() const
    {
        return getPieces(WHITE) | getPieces(BLACK);
    }

    bool isSideInCheck(PieceColor side) const
    {
        Bitboard kingBitboard = side == WHITE ? whiteKing : blackKing;
        Bitboard squaresAttackedBySide = getAttackingSquares(oppositeColor(side));
        return (squaresAttackedBySide & kingBitboard) != 0;
    }

    bool isCheck() const
    {
        return isSideInCheck(WHITE) || isSideInCheck(BLACK);
    }

    bool isCheckmate(PieceColor side)
    {
        return sideToMove == side && isSideInCheck(side) && getLegalMoves().empty();
    }

    Piece operator[](Square index) const
    {
        return board[index];
    }

    int8_t getEnPassantTargetSquare() const
    {
        return enPassantTargetSquare;
    }

    Bitboard getAttackingSquares(PieceColor side) const
    {
        return side == WHITE ? whiteAttackingSquares : blackAttackingSquares;
    }

    Bitboard getPawnAttackingSquares(PieceColor side) const
    {
        return side == WHITE ? whitePawnAttackingSquares : blackPawnAttackingSquares;
    }

    bool isSquareEmpty(Square square) const
    {
        return board[square].kind == PieceKind::NONE;
    }

    bool canWhiteShortCastle() const
    {
        return whiteCanShortCastle;
    }

    bool canWhiteLongCastle() const
    {
        return whiteCanLongCastle;
    }

    bool canBlackShortCastle() const
    {
        return blackCanShortCastle;
    }

    bool canBlackLongCastle() const
    {
        return blackCanLongCastle;
    }

    uint64_t getHash() const
    {
        return hashHistory.top();
    }

    bool isStalemate();
    bool isInsufficientMaterial() const;
    bool isThreefoldRepetition();

private:
    std::array<Piece, 64> board{};

    Bitboard whiteAttackingSquares = 0;
    Bitboard blackAttackingSquares = 0;

    Bitboard whitePawnAttackingSquares = 0;
    Bitboard blackPawnAttackingSquares = 0;

    int8_t enPassantTargetSquare = -1;

    bool whiteCanShortCastle = false;
    bool whiteCanLongCastle = false;
    bool blackCanShortCastle = false;
    bool blackCanLongCastle = false;

    uint8_t halfMoveClock = 0;

    std::vector<Move> moveHistory;
    std::stack<uint64_t> hashHistory;
    std::stack<BoardState> boardHistory;

    uint64_t hash() const;
    uint64_t hashAfterMove(Move move, Piece movingPiece, Piece capturedPiece, uint64_t currentHash) const;

    void movePiece(Piece piece, Piece capturedPiece, Square start, Square end);
    void addPiece(Piece piece, Square position);
    void addPiece(MoveFlag promotedPiece, PieceColor side, Square position);
    void removePiece(Piece piece, Square position);
    void removePiece(MoveFlag promotedPiece, PieceColor side, Square position);
    void updateAttackingSquares();
};
