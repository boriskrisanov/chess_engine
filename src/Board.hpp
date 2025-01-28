#pragma once

#include <array>
#include <stack>
#include <string>

#include "bitboards.hpp"
#include "Piece.hpp"

const std::string STARTING_POSITION_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    std::array<Bitboard, 14> bitboards{};
    void loadFen(std::string fen);
    std::string getFen() const;
    std::string getPgn();
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

    PieceColor sideToMove = PieceColor::WHITE;
    Square whiteKingPosition = 0;
    Square blackKingPosition = 0;

    operator std::string() const
    {
        return toString();
    }

    Bitboard getPieces(PieceColor color) const
    {

        using enum PieceKind;
        using enum PieceColor;
        return color == WHITE
                   ? bitboards[Piece{PAWN, WHITE}.index()] |
                   bitboards[Piece{KNIGHT, WHITE}.index()] |
                   bitboards[Piece{BISHOP, WHITE}.index()] |
                   bitboards[Piece{ROOK, WHITE}.index()] |
                   bitboards[Piece{QUEEN, WHITE}.index()] |
                   bitboards[Piece{KING, WHITE}.index()]
                   : bitboards[Piece{PAWN, BLACK}.index()] |
                   bitboards[Piece{KNIGHT, BLACK}.index()] |
                   bitboards[Piece{BISHOP, BLACK}.index()] |
                   bitboards[Piece{ROOK, BLACK}.index()] |
                   bitboards[Piece{QUEEN, BLACK}.index()] |
                   bitboards[Piece{KING, BLACK}.index()];
    }

    Bitboard getPieces() const
    {
        return getPieces(PieceColor::WHITE) | getPieces(PieceColor::BLACK);
    }

    bool isSideInCheck(PieceColor side) const
    {
        Bitboard kingBitboard = side == PieceColor::WHITE
                                    ? bitboards[Piece{PieceKind::KING, PieceColor::WHITE}.index()]
                                    : bitboards[Piece{PieceKind::KING, PieceColor::BLACK}.index()];
        Bitboard squaresAttackedBySide = getAttackingSquares(oppositeColor(side));
        return (squaresAttackedBySide & kingBitboard) != 0;
    }

    bool isCheck() const
    {
        return isSideInCheck(PieceColor::WHITE) || isSideInCheck(PieceColor::BLACK);
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
        return side == PieceColor::WHITE ? whiteAttackingSquares : blackAttackingSquares;
    }

    Bitboard getPawnAttackingSquares(PieceColor side) const
    {
        return side == PieceColor::WHITE ? whitePawnAttackingSquares : blackPawnAttackingSquares;
    }

    bool isSquareEmpty(Square square) const
    {
        return board[square].isNone();
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

    std::vector<Move> getMoveHistory()
    {
        return moveHistory;
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
