#pragma once

#include <cctype>
#include <string>
#include <cstdint>
#include "MoveFlag.hpp"

enum class PieceKind : uint8_t
{
    PAWN = 0x00,
    KNIGHT = 0x01,
    BISHOP = 0x02,
    ROOK = 0x03,
    QUEEN = 0x04,
    KING = 0x05,
    NONE = 0xFF
};

enum class PieceColor : uint8_t
{
    WHITE = 0,
    BLACK = 0b00001000
};

// inline PieceKind pieceKind(Piece piece)
// {
//     return piece & 0b00000111;
// }
//
// inline PieceColor pieceColor(Piece piece)
// {
//     return piece & 0b00001000;
// }
//
inline PieceColor oppositeColor(PieceColor color)
{
    return static_cast<PieceColor>(static_cast<uint8_t>(color) ^ 0b00001000);
}

//
// inline bool isNone(Piece piece)
// {
//     return pieceKind(piece) == NONE;
// }

struct Piece
{
    Piece(PieceKind kind, PieceColor color)
        : data(static_cast<uint8_t>(kind) | static_cast<uint8_t>(color))
    {
    }

    Piece() = default;

    // TODO: Cannot use kind() == NONE because of 0xFF definition
    PieceKind kind() const
    {
        return static_cast<PieceKind>(data & 0b00000111);
    }

    PieceColor color() const
    {
        return static_cast<PieceColor>(data & 0b00001000);
    }

    bool isNone() const
    {
        return data == 0xFF;
    }

    bool isSlidingPiece() const
    {
        return kind() == PieceKind::BISHOP || kind() == PieceKind::ROOK || kind() == PieceKind::QUEEN;
    }

    explicit Piece(char c)
    {
        using enum PieceKind;
        using enum PieceColor;

        data = static_cast<uint8_t>(isupper(c) ? WHITE : BLACK);
        switch (tolower(c))
        {
        case 'p':
            data |= static_cast<uint8_t>(PAWN);
            break;
        case 'n':
            data |= static_cast<uint8_t>(KNIGHT);
            break;
        case 'b':
            data |= static_cast<uint8_t>(BISHOP);
            break;
        case 'r':
            data |= static_cast<uint8_t>(ROOK);
            break;
        case 'q':
            data |= static_cast<uint8_t>(QUEEN);
            break;
        case 'k':
            data |= static_cast<uint8_t>(KING);
            break;
        default:
            data |= static_cast<uint8_t>(NONE);
        }
    }

    Piece(MoveFlag promotion, PieceColor side)
    {
        using enum MoveFlag;
        using enum PieceKind;
        PieceKind kind = NONE;
        switch (promotion)
        {
        case PromotionQueen:
            kind = QUEEN;
            break;
        case PromotionRook:
            kind = ROOK;
            break;
        case PromotionBishop:
            kind = BISHOP;
            break;
        case PromotionKnight:
            kind = KNIGHT;
            break;
        default:
            data = static_cast<uint8_t>(NONE);
        }
        data = static_cast<uint8_t>(kind) | static_cast<uint8_t>(side);
    }

    std::string toString() const
    {
        using enum PieceKind;
        using enum PieceColor;
        switch (kind())
        {
        case PAWN:
            return color() == WHITE ? "P" : "p";
        case KNIGHT:
            return color() == WHITE ? "N" : "n";
        case BISHOP:
            return color() == WHITE ? "B" : "b";
        case ROOK:
            return color() == WHITE ? "R" : "r";
        case QUEEN:
            return color() == WHITE ? "Q" : "q";
        case KING:
            return color() == WHITE ? "K" : "k";
        default:
            return "";
        }
    }

    uint8_t index() const
    {
        return data;
    }

private:
    uint8_t data = static_cast<uint8_t>(PieceKind::NONE);
};

namespace pieceIndexes
{
    const uint8_t WHITE_PAWN = Piece{PieceKind::PAWN, PieceColor::WHITE}.index();
    const uint8_t WHITE_KNIGHT = Piece{PieceKind::KNIGHT, PieceColor::WHITE}.index();
    const uint8_t WHITE_BISHOP = Piece{PieceKind::BISHOP, PieceColor::WHITE}.index();
    const uint8_t WHITE_ROOK = Piece{PieceKind::ROOK, PieceColor::WHITE}.index();
    const uint8_t WHITE_QUEEN = Piece{PieceKind::QUEEN, PieceColor::WHITE}.index();
    const uint8_t WHITE_KING = Piece{PieceKind::KING, PieceColor::WHITE}.index();

    const uint8_t BLACK_PAWN = Piece{PieceKind::PAWN, PieceColor::BLACK}.index();
    const uint8_t BLACK_KNIGHT = Piece{PieceKind::KNIGHT, PieceColor::BLACK}.index();
    const uint8_t BLACK_BISHOP = Piece{PieceKind::BISHOP, PieceColor::BLACK}.index();
    const uint8_t BLACK_ROOK = Piece{PieceKind::ROOK, PieceColor::BLACK}.index();
    const uint8_t BLACK_QUEEN = Piece{PieceKind::QUEEN, PieceColor::BLACK}.index();
    const uint8_t BLACK_KING = Piece{PieceKind::KING, PieceColor::BLACK}.index();
}
