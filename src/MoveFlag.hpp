#pragma once

#include <cstdint>

enum class MoveFlag : uint8_t
{
    None,
    EnPassant,
    ShortCastling,
    LongCastling,
    PromotionKnight,
    PromotionBishop,
    PromotionRook,
    PromotionQueen
};
