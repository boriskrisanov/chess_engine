#include "Move.hpp"

Move::Move(Square start, Square end, MoveFlag flag)
{
    moveData |= start << 10;
    moveData |= end << 4;
    moveData |= static_cast<uint8_t>(flag);
}

uint8_t Move::start() const
{
    return (moveData & 0b1111110000000000) >> 10;
}

uint8_t Move::end() const
{
    return (moveData & 0b0000001111110000) >> 4;
}

MoveFlag Move::moveFlag() const
{
    return static_cast<MoveFlag>(moveData & 0b0000000000001111);
}
