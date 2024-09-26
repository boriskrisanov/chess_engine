#pragma once

#include <cctype>
#include <string>
#include <type_traits>

enum class PieceKind : uint8_t
{
    NONE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum class PieceColor
{
    WHITE,
    BLACK
};

inline PieceColor oppositeColor(PieceColor color)
{
    return color == PieceColor::WHITE ? PieceColor::BLACK : PieceColor::WHITE;
}

struct Piece
{
    PieceKind kind = PieceKind::NONE;
    PieceColor color = PieceColor::WHITE;
    bool isSlidingPiece = false;

    bool isNone() const
    {
        return kind == PieceKind::NONE;
    }

    Piece() = default;

    Piece(PieceKind kind, PieceColor color)
        : kind(kind), color(color), isSlidingPiece(kind == PieceKind::ROOK || kind == PieceKind::BISHOP || kind == PieceKind::QUEEN)
    {

    }

    explicit Piece(char c)
    {
        color = isupper(c) ? PieceColor::WHITE : PieceColor::BLACK;
        switch (tolower(c))
        {
        case 'p':
            kind = PieceKind::PAWN;
            break;
        case 'n':
            kind = PieceKind::KNIGHT;
            break;
        case 'b':
            kind = PieceKind::BISHOP;
            break;
        case 'r':
            kind = PieceKind::ROOK;
            break;
        case 'q':
            kind = PieceKind::QUEEN;
            break;
        case 'k':
            kind = PieceKind::KING;
            break;
        default:
            kind = PieceKind::NONE;
        }

        isSlidingPiece = kind == PieceKind::ROOK || kind == PieceKind::BISHOP || kind == PieceKind::QUEEN;
    }

    std::string toString() const
    {
        switch (kind)
        {
        case PieceKind::PAWN:
            return color == PieceColor::WHITE ? "P" : "p";
        case PieceKind::KNIGHT:
            return color == PieceColor::WHITE ? "N" : "n";
        case PieceKind::BISHOP:
            return color == PieceColor::WHITE ? "B" : "b";
        case PieceKind::ROOK:
            return color == PieceColor::WHITE ? "R" : "r";
        case PieceKind::QUEEN:
            return color == PieceColor::WHITE ? "Q" : "q";
        case PieceKind::KING:
            return color == PieceColor::WHITE ? "K" : "k";
        default:
            return "";
        }
    }
};
