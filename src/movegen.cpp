#include "movegen.hpp"
#include "Board.hpp"
#include "bitboards.hpp"
#include <bit>

using std::vector, std::array;
using enum PieceColor;

namespace movegen
{
enum class Direction : int8_t
{
    NORTH = -8,
    SOUTH = 8,
    WEST = -1,
    EAST = 1,
    NORTHWEST = NORTH + WEST,
    NORTHEAST = NORTH + EAST,
    SOUTHWEST = SOUTH + WEST,
    SOUTHEAST = SOUTH + EAST
};

constexpr array<EdgeDistance, 64> edgeDistances = []() consteval
{
    array<EdgeDistance, 64> distances{};

    for (int i = 0; i < 64; i++)
    {
        const Square rank = i / 8 + 1;
        auto &[west, east, north, south, northwest, northeast, southwest, southeast] = distances[i];

        west = 8 - (rank * 8 - i);
        east = rank * 8 - i - 1;
        north = i / 8;
        south = (63 - i) / 8;
        northwest = std::min(north, west);
        northeast = std::min(north, east);
        southwest = std::min(south, west);
        southeast = std::min(south, east);
    }

    return distances;
}();

array<EdgeDistance, 64> getEdgeDistances()
{
    return edgeDistances;
}

constexpr uint8_t edgeDistanceInDirection(Square square, Direction direction)
{
    switch (direction)
    {
    case Direction::NORTH:
        return edgeDistances[square].NORTH;
    case Direction::SOUTH:
        return edgeDistances[square].SOUTH;
    case Direction::WEST:
        return edgeDistances[square].WEST;
    case Direction::EAST:
        return edgeDistances[square].EAST;
    case Direction::NORTHWEST:
        return edgeDistances[square].NORTHWEST;
    case Direction::NORTHEAST:
        return edgeDistances[square].NORTHEAST;
    case Direction::SOUTHWEST:
        return edgeDistances[square].SOUTHWEST;
    case Direction::SOUTHEAST:
        return edgeDistances[square].SOUTHEAST;
    }
    std::unreachable();
}

constexpr Bitboard rayAttackingSquares(Bitboard blockers, Square position, const vector<Direction> &directions)
{
    Bitboard attackingSquares = 0;

    for (Direction direction : directions)
    {
        for (int i = 0; i < edgeDistanceInDirection(position, direction); i++)
        {
            const int targetSquare = position + static_cast<int8_t>(direction) * (i + 1);
            attackingSquares |= bitboards::withSquare(targetSquare);

            if ((blockers & bitboards::withSquare(targetSquare)) != 0)
            {
                break;
            }
        }
    }

    return attackingSquares;
}

constexpr array<Bitboard, 64> knightAttackingSquares = []() consteval
{
    array<Bitboard, 64> bitboards{};
    for (Square i = 0; i < 64; i++)
    {
        Bitboard squares = 0;
        const auto edgeDistance = edgeDistances[i];
        if (edgeDistance.WEST >= 2 && edgeDistance.NORTH >= 1)
        {
            squares |= bitboards::withSquare(i - 8 - 2);
        }
        if (edgeDistance.WEST >= 1 && edgeDistance.NORTH >= 2)
        {
            squares |= bitboards::withSquare(i - 8 * 2 - 1);
        }
        if (edgeDistance.EAST >= 1 && edgeDistance.NORTH >= 2)
        {
            squares |= bitboards::withSquare(i - 8 * 2 + 1);
        }
        if (edgeDistance.WEST >= 2 && edgeDistance.SOUTH >= 1)
        {
            squares |= bitboards::withSquare(i - 2 + 8);
        }
        if (edgeDistance.EAST >= 2 && edgeDistance.SOUTH >= 1)
        {
            squares |= bitboards::withSquare(i + 2 + 8);
        }
        if (edgeDistance.WEST >= 1 && edgeDistance.SOUTH >= 2)
        {
            squares |= bitboards::withSquare(i + 8 * 2 - 1);
        }
        if (edgeDistance.EAST >= 1 && edgeDistance.SOUTH >= 2)
        {
            squares |= bitboards::withSquare(i + 8 * 2 + 1);
        }
        if (edgeDistance.EAST >= 2 && edgeDistance.NORTH >= 1)
        {
            squares |= bitboards::withSquare(i - 8 + 2);
        }
        bitboards[i] = squares;
    }
    return bitboards;
}();

constexpr array<Bitboard, 64> kingAttackingSquares = []() consteval
{
    using namespace bitboards;
    array<Bitboard, 64> bitboards{};
    for (Square i = 0; i < 64; i++)
    {
        Bitboard squares = 0;
        const Bitboard king = withSquare(i);
        squares |= (king & ~RANK_8) << 8;
        squares |= (king & ~RANK_1) >> 8;
        squares |= (king & ~FILE_A) << 1;
        squares |= (king & ~FILE_H) >> 1;
        squares |= (king & ~RANK_8 & ~FILE_A) << 9;
        squares |= (king & ~RANK_8 & ~FILE_H) << 7;
        squares |= (king & ~RANK_1 & ~FILE_A) >> 7;
        squares |= (king & ~RANK_1 & ~FILE_H) >> 9;
        bitboards[i] = squares;
    }
    return bitboards;
}();

struct RayFromSquare
{
    Bitboard bitboard;
    Direction direction;
};

constexpr array<array<RayFromSquare, 8>, 64> precomputeSquareRays()
{
    using enum Direction;
    array<array<RayFromSquare, 8>, 64> rays{};

    for (Square i = 0; i < 64; i++)
    {
        rays[i][0] = {rayAttackingSquares(0, i, {NORTH}), NORTH};
        rays[i][1] = {rayAttackingSquares(0, i, {SOUTH}), SOUTH};
        rays[i][2] = {rayAttackingSquares(0, i, {WEST}), WEST};
        rays[i][3] = {rayAttackingSquares(0, i, {EAST}), EAST};
        rays[i][4] = {rayAttackingSquares(0, i, {NORTHWEST}), NORTHWEST};
        rays[i][5] = {rayAttackingSquares(0, i, {NORTHEAST}), NORTHEAST};
        rays[i][6] = {rayAttackingSquares(0, i, {SOUTHWEST}), SOUTHWEST};
        rays[i][7] = {rayAttackingSquares(0, i, {SOUTHEAST}), SOUTHEAST};
    }

    return rays;
}

const auto SQUARE_RAYS = precomputeSquareRays();

array<array<Bitboard, 64>, 64> computeSquaresBetweenSquares()
{
    array<array<Bitboard, 64>, 64> s{};
    for (Square i = 0; i < 64; i++)
    {
        for (const auto [rayBitboard, direction] : SQUARE_RAYS[i])
        {
            for (Square j = 0; j < edgeDistanceInDirection(i, direction); j++)
            {
                const int offset = static_cast<int>(direction);
                const Square targetSquare = i + offset * (j + 1);
                Square k = i + offset;
                while (k != targetSquare)
                {
                    s[i][targetSquare] |= bitboards::withSquare(k);
                    k += offset;
                }
            }
        }
    }
    return s;
}

const array<array<Bitboard, 64>, 64> squaresBetweenSquares = computeSquaresBetweenSquares();

vector<Bitboard> possibleBlockerPositions(Bitboard blockerMask)
{
    vector<Bitboard> configurations{};
    vector<uint8_t> indexes{};
    for (int i = 0; i < 64; i++)
    {
        if ((1ULL << i & blockerMask) != 0)
        {
            indexes.push_back(63 - i);
        }
    }

    // 2^k possible blocker configurations
    const int n = 1 << std::popcount(blockerMask);
    for (int configuration = 0; configuration < n; configuration++)
    {
        int j = 0;
        Bitboard finalConfig = 0;
        for (int i = 0; i < std::popcount(blockerMask); i++)
        {
            if (((configuration >> i) & 1) != 0)
            {
                finalConfig |= static_cast<Bitboard>(1) << (63 - indexes.at(j));
            }
            j++;
        }
        configurations.push_back(finalConfig);
    }

    return configurations;
}

constexpr array<uint64_t, 64> ROOK_MAGICS{
    0xe7ab6c0052e686be, 0x14087c57effa2114, 0x9a789e001704734e, 0xc1289dffdaf1f9ee, 0x7f30e5fff60cacce,
    0x389f1339a5c1d9f2, 0xccc6afd1ffed5a56, 0xf9f4de4121b50082, 0xf703530ca9440200, 0x5451028464f1c00,
    0x146926f02333020, 0x1de0682c5de44bb4, 0x53c44ca53b3ffbc8, 0x75d17116b393dce7, 0x420a483a1011483c,
    0x504e20b1235f7418, 0x2d3a8407814e0011, 0xbcea30087204004f, 0xfbfe0b7023bfd448, 0x7c597408a4300118,
    0xdf6bd66fd7a77aad, 0x71ec963e5034152f, 0xaab1beeb167c2ee1, 0x959cfcfcb06ff9ff, 0xa1efe6a1b470a33e,
    0x46788c2411000ea2, 0xe181860941861e6e, 0x2a50d993366ca4bf, 0xab177c0005a5a72d, 0x606658f9e8f547b3,
    0xcf0c96cf5b5de517, 0x36ac6caf5cd7fdaf, 0x779d5ff5fd0134c9, 0xcfeedc6af2073398, 0x94d8200440150426,
    0xa70cdbd37373759d, 0xc180c5e0c5828ea7, 0x2db7729c47680043, 0x18e4424791d4540, 0x536ae2b28f5b2e04,
    0x53eb0f488c94a5a2, 0xc94e554fd5335613, 0x26ba46ab90f23543, 0x94684d970e376eb3, 0x45448c2e80106a17,
    0xde0850180478aee7, 0xdbe5f061f0dab6f1, 0xaf9aed4bf7af67c2, 0x4a28180ea295b306, 0x75308266af7918f9,
    0x49f30390d1387303, 0x9278ca1e302e18ca, 0xe3006f606fd7d14f, 0x67881683707d30e5, 0x1a3ae0f34e13dc3f,
    0x81eb3e9f45ee2418, 0x4d990672c598c195, 0xc0044c7fc09f0007, 0x13e1f731196ffa22, 0xd5a20cbca7067bc4,
    0x1b139d8d4539b139, 0xde87571a5615ade, 0xff6fe8ffc19e2a91, 0x6905474a0c1585};
constexpr array<uint64_t, 64> ROOK_SHIFTS{
    52, 52, 52, 52, 52, 52, 52, 51, 53, 54, 53, 53, 53, 53, 53, 52, 53, 54, 52, 53, 53, 53, 53, 52, 52, 53, 53, 53,
    53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52, 52, 53, 53, 52, 53, 53, 53, 52, 52, 53, 53, 53, 53, 53, 53, 52,
    51, 52, 51, 51, 52, 51, 51, 50};

constexpr array<uint64_t, 64> BISHOP_MAGICS{
    0x85380e2c592c4fa1, 0x11182de0338c6980, 0xd6868fee200816cf, 0xbfa5e4050120619, 0x186ca2437de19811,
    0x691909c291480803, 0x2279d6ef2b10171c, 0x11e4fa7048d410c8, 0x76b5500a1a01a033, 0x4c303010009fa349,
    0x7f5700ab0310263, 0x754305ec8504003f, 0x626b3c21e0982c0d, 0x56436a02ccb5cf47, 0xf341074f04203fb7,
    0xe80b83a808c02b97, 0xe7680a9c06427781, 0xd7c8c258bdfba3f4, 0xa64522de33d187a3, 0xf0540a0d9543255c,
    0x8ed13334d81069a0, 0xa629c2034047c404, 0x6fb5b52d36e52003, 0x3538024650f9e07e, 0x1e38034380c30b8a,
    0x525280b098b9420, 0xf6ee878303ae0105, 0x24e2b7c812543d07, 0xb6ce1af139079792, 0xee8c00609f31f031,
    0xa461286012880d0d, 0x8368844002a42823, 0xea02328aa600865c, 0x1b7bf8eab2b7e9ac, 0x99010a0085005d0d,
    0x58da7050f7d6cb66, 0x4a1853a2e747ae89, 0x9d38a6033904f141, 0x85b8242fce304e0a, 0x94d04009901c13aa,
    0x107e16f396062a27, 0x7fc1024c088854bd, 0x8bfe3d8c62b2074f, 0x239d829c00a056b8, 0x9d369feb48b26a94,
    0xeec7fdeff5770c70, 0x85094c1218080087, 0x61c032dc08c80706, 0x56467c84041f2444, 0x72d6d20822221121,
    0xec34ee059b2232a2, 0x98300910c1069902, 0x40d21816084174f5, 0xefde98298c1387ce, 0xb4c1cc8428124f01,
    0x8d5db872b0620202, 0x5b4f9027194cccb0, 0x4a9c0509b03038e6, 0xe24e033ca04efc19, 0x61d40ce056a552a2,
    0xb2bc241386917c17, 0x81902588910bf33f, 0x6a045c4c034a077a, 0x40a104f4db56b65c};
constexpr array<uint64_t, 64> BISHOP_SHIFTS{
    57, 59, 59, 59, 59, 59, 59, 57, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 56, 56, 56, 57, 59, 59, 59, 59, 57, 53,
    53, 56, 59, 59, 59, 59, 57, 53, 54, 56, 59, 59, 59, 59, 56, 57, 56, 56, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59,
    57, 59, 59, 59, 59, 59, 59, 57};

array<Bitboard, 64> ROOK_BLOCKER_MASKS = []
{
    array<Bitboard, 64> masks{};
    for (int rookIndex = 0; rookIndex < 64; rookIndex++)
    {
        for (int i = 0; i < 64; i++)
        {
            if (i != rookIndex && (square::rank(i) == square::rank(rookIndex) || square::file(i) ==
                                                                                     square::file(rookIndex)))
            {
                // Don't add the square if it's on the edge
                if ((square::file(i) == 1 && square::file(rookIndex) != 1) || (square::file(i) == 8 &&
                                                                               square::file(rookIndex) != 8))
                {
                    continue;
                }
                if ((square::rank(i) == 1 && square::rank(rookIndex) != 1) || (square::rank(i) == 8 &&
                                                                               square::rank(rookIndex) != 8))
                {
                    continue;
                }

                masks[rookIndex] |= bitboards::withSquare(i);
            }
        }
    }
    return masks;
}();

array<Bitboard, 64> BISHOP_BLOCKER_MASKS = []
{
    array<Bitboard, 64> masks{};
    for (int bishopIndex = 0; bishopIndex < 64; bishopIndex++)
    {
        constexpr array directions{
            Direction::NORTHWEST, Direction::NORTHEAST, Direction::SOUTHWEST, Direction::SOUTHEAST};
        for (Direction direction : directions)
        {
            for (int i = 0; i < edgeDistanceInDirection(bishopIndex, direction); i++)
            {
                const Square targetSquare = bishopIndex + static_cast<uint8_t>(direction) * (i + 1);
                const bool isEdgeSquare = square::rank(targetSquare) == 1 || square::rank(targetSquare) == 8 ||
                                          square::file(targetSquare) == 1 || square::file(targetSquare) == 8;
                if (!isEdgeSquare)
                {
                    masks[bishopIndex] |= bitboards::withSquare(targetSquare);
                }
            }
        }
    }
    return masks;
}();

// TODO: Use consteval for magic bitboard functions that can run at compile time
array<vector<Bitboard>, 64> computeRookAttackingSquares()
{
    array<vector<Bitboard>, 64> attackingSquares{};

    for (Square i = 0; i < 64; i++)
    {
        uint64_t arrayLength = 0;
        for (const Bitboard blockerMask : possibleBlockerPositions(ROOK_BLOCKER_MASKS[i]))
        {
            arrayLength = std::max(arrayLength, blockerMask * ROOK_MAGICS[i] >> ROOK_SHIFTS[i]);
        }
        // Add 1 because the array length will be one more than the maximum index
        arrayLength++;

        attackingSquares[i].resize(arrayLength);
        for (const Bitboard blockerPositions : possibleBlockerPositions(ROOK_BLOCKER_MASKS[i]))
        {
            const uint32_t index = blockerPositions * ROOK_MAGICS[i] >> ROOK_SHIFTS[i];
            attackingSquares[i][index] = rayAttackingSquares(blockerPositions, i, {Direction::NORTH, Direction::SOUTH, Direction::WEST, Direction::EAST});
        }
    }

    return attackingSquares;
}

// TODO: Merge with rook attacking squares
array<vector<Bitboard>, 64> computeBishopAttackingSquares()
{
    array<vector<Bitboard>, 64> attackingSquares{};

    for (Square i = 0; i < 64; i++)
    {
        uint64_t arrayLength = 0;
        for (const Bitboard blockerMask : possibleBlockerPositions(BISHOP_BLOCKER_MASKS[i]))
        {
            arrayLength = std::max(arrayLength, blockerMask * BISHOP_MAGICS[i] >> BISHOP_SHIFTS[i]);
        }
        // Add 1 because the array length will be one more than the maximum index
        arrayLength++;

        attackingSquares[i].resize(arrayLength);
        for (const Bitboard blockerPositions : possibleBlockerPositions(BISHOP_BLOCKER_MASKS[i]))
        {
            const uint32_t index = blockerPositions * BISHOP_MAGICS[i] >> BISHOP_SHIFTS[i];
            attackingSquares[i][index] = rayAttackingSquares(blockerPositions, i, {Direction::NORTHWEST, Direction::NORTHEAST, Direction::SOUTHWEST, Direction::SOUTHEAST});
        }
    }

    return attackingSquares;
}

// TODO: Use array instead of vector (array of pointers to arrays of different lengths)
const array<vector<Bitboard>, 64> ROOK_ATTACKING_SQUARES = computeRookAttackingSquares();
const array<vector<Bitboard>, 64> BISHOP_ATTACKING_SQUARES = computeBishopAttackingSquares();

Bitboard slidingCheckers = 0;

Bitboard checkResolutionSquares(const Board &board)
{
    using enum Direction;
    using pieceIndexes::WHITE_PAWN, pieceIndexes::BLACK_PAWN;

    Bitboard slidingCheckEvasions = 0;
    const Bitboard king = board.bitboards[Piece{PieceKind::KING, board.sideToMove}.index()];
    const Square kingPos = bitboards::getMSB(king);
    // Sliding pieces
    if (std::popcount(slidingCheckers) > 1)
    {
        // Double check, king must move
        return 0;
    }
    if (slidingCheckers != 0)
    {
        slidingCheckEvasions = squaresBetweenSquares[kingPos][bitboards::getMSB(slidingCheckers)] |
                               bitboards::withSquare(
                                   bitboards::getMSB(slidingCheckers));
    }

    Bitboard nonSlidingCheckers = 0;
    // Pawns
    if (board.sideToMove == WHITE)
    {
        nonSlidingCheckers |= ((king & ~bitboards::FILE_A) << 9) & board.bitboards[BLACK_PAWN];
        nonSlidingCheckers |= ((king & ~bitboards::FILE_H) << 7) & board.bitboards[BLACK_PAWN];
    }
    else
    {
        nonSlidingCheckers |= ((king & ~bitboards::FILE_H) >> 9) & board.bitboards[WHITE_PAWN];
        nonSlidingCheckers |= ((king & ~bitboards::FILE_A) >> 7) & board.bitboards[WHITE_PAWN];
    }

    // Knights
    const Bitboard attackingKnights = knightAttackingSquares[kingPos] & board.bitboards[Piece{
                                                                            PieceKind::KNIGHT, oppositeColor(board.sideToMove)}
                                                                                            .index()];
    nonSlidingCheckers |= attackingKnights;

    if (slidingCheckers != 0 && nonSlidingCheckers != 0)
    {
        // Discovered double check, there are no resolution squares because it cannot be blocked and both pieces
        // cannot be captured in one move.
        return 0;
    }
    return slidingCheckEvasions | nonSlidingCheckers;
}

array<Bitboard, 64> pinLines{};
// This should be called before checkResolutionSquares()
void computePinLinesAndSlidingCheckers(const Board &board, PieceColor side)
{
    using enum Direction;

    pinLines.fill(bitboards::ALL_SQUARES);
    slidingCheckers = 0;

    const Bitboard king = board.bitboards[Piece{PieceKind::KING, side}.index()];
    const Bitboard enemyRooks = board.bitboards[Piece{PieceKind::ROOK, oppositeColor(side)}.index()];
    const Bitboard enemyBishops = board.bitboards[Piece{PieceKind::BISHOP, oppositeColor(side)}.index()];
    const Bitboard enemyQueens = board.bitboards[Piece{PieceKind::QUEEN, oppositeColor(side)}.index()];
    const Square kingPos = bitboards::getMSB(king);

    for (const auto [rayBitboard, direction] : SQUARE_RAYS[kingPos])
    {
        const Bitboard orthogonalSliders = rayBitboard & (enemyRooks | enemyQueens);
        const Bitboard diagonalSliders = rayBitboard & (enemyBishops | enemyQueens);
        const Bitboard possibleAttackers =
            direction == NORTH || direction == SOUTH || direction == WEST || direction == EAST
                ? orthogonalSliders
                : diagonalSliders;
        if (possibleAttackers == 0)
        {
            // Cannot be pinned on this ray
            continue;
        }
        // Get the first attacker along this direction. If the index of the attacker is less than the king, this will be the least significant bit. If it is greater, this will be the most significant bit.
        const Square attackerPos = (direction == NORTH || direction == WEST || direction == NORTHWEST || direction == NORTHEAST)
                                       ? bitboards::getLSB(possibleAttackers)
                                       : bitboards::getMSB(possibleAttackers);
        Bitboard piecesBetweenKingAndAttacker = squaresBetweenSquares[kingPos][attackerPos] & board.getPieces() & ~possibleAttackers;
        if (std::popcount(piecesBetweenKingAndAttacker) == 0)
        {
            // In check from this direction
            slidingCheckers |= bitboards::withSquare(attackerPos);
            continue;
        }
        if (std::popcount(piecesBetweenKingAndAttacker) == 1)
        {
            Bitboard friendlyIntersectingPiece = piecesBetweenKingAndAttacker & board.getPieces(side);
            if (friendlyIntersectingPiece != 0)
            {
                // Piece is pinned
                pinLines[bitboards::getMSB(friendlyIntersectingPiece)] = squaresBetweenSquares[kingPos][attackerPos] | bitboards::withSquare(attackerPos);
            }
        }
        // More than 2 pieces between attacker and king, no pinned pieces on this ray
    }
}

void generatePawnMoves(MoveList &moves, Board &board, Bitboard checkResolutions)
{
    const PieceColor side = board.sideToMove;
    const Bitboard pawns = board.bitboards[Piece{PieceKind::PAWN, side}.index()];
    const Bitboard emptySquares = ~board.getPieces();
    const Bitboard enemyPieces = board.getPieces(oppositeColor(side));
    const int direction = side == WHITE ? 1 : -1;

    const Bitboard doublePushTarget = side == WHITE ? bitboards::RANK_4 : bitboards::RANK_5;
    const Bitboard promotionRank = side == WHITE ? bitboards::RANK_8 : bitboards::RANK_1;
    Bitboard singlePushes = (side == WHITE ? pawns << 8 : pawns >> 8) & emptySquares;
    Bitboard doublePushes = (side == WHITE ? singlePushes << 8 : singlePushes >> 8) & emptySquares &
                            doublePushTarget;
    Bitboard leftCaptures = (side == WHITE
                                 ? (pawns & ~bitboards::FILE_A) << 9
                                 : (pawns & ~bitboards::FILE_A) >> 7) &
                            enemyPieces;
    Bitboard rightCaptures = (side == WHITE
                                  ? (pawns & ~bitboards::FILE_H) << 7
                                  : (pawns & ~bitboards::FILE_H) >> 9) &
                             enemyPieces;

    Bitboard singlePushesWithPromotion = singlePushes & promotionRank;
    Bitboard leftCapturesWithPromotion = leftCaptures & promotionRank;
    Bitboard rightCapturesWithPromotion = rightCaptures & promotionRank;

    singlePushes &= ~promotionRank;
    leftCaptures &= ~promotionRank;
    rightCaptures &= ~promotionRank;

    // Pawn pushes
    while (singlePushes != 0)
    {
        const Square i = bitboards::popMSB(singlePushes);
        const Square start = i + 8 * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::None);
        }
    }
    while (doublePushes != 0)
    {
        const Square i = bitboards::popMSB(doublePushes);
        const Square start = i + 16 * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::None);
        }
    }

    // Captures
    while (leftCaptures != 0)
    {
        Square i = bitboards::popMSB(leftCaptures);
        const Square start = i + (side == WHITE ? 9 : 7) * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::None);
        }
    }
    while (rightCaptures != 0)
    {
        Square i = bitboards::popMSB(rightCaptures);
        const Square start = i + (side == WHITE ? 7 : 9) * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::None);
        }
    }

    // Promotion
    while (singlePushesWithPromotion != 0)
    {
        const Square i = bitboards::popMSB(singlePushesWithPromotion);
        const Square start = i + 8 * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::PromotionQueen);
            moves.emplace_back(start, i, MoveFlag::PromotionRook);
            moves.emplace_back(start, i, MoveFlag::PromotionBishop);
            moves.emplace_back(start, i, MoveFlag::PromotionKnight);
        }
    }
    while (leftCapturesWithPromotion != 0)
    {
        Square i = bitboards::popMSB(leftCapturesWithPromotion);
        const Square start = i + (side == WHITE ? 9 : 7) * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::PromotionQueen);
            moves.emplace_back(start, i, MoveFlag::PromotionRook);
            moves.emplace_back(start, i, MoveFlag::PromotionBishop);
            moves.emplace_back(start, i, MoveFlag::PromotionKnight);
        }
    }
    while (rightCapturesWithPromotion != 0)
    {
        const Square i = bitboards::popMSB(rightCapturesWithPromotion);
        const Square start = i + (side == WHITE ? 7 : 9) * direction;
        const Bitboard targetBitboard = bitboards::withSquare(i);
        if ((checkResolutions & targetBitboard) != 0 && (pinLines[start] & targetBitboard) != 0)
        {
            moves.emplace_back(start, i, MoveFlag::PromotionQueen);
            moves.emplace_back(start, i, MoveFlag::PromotionRook);
            moves.emplace_back(start, i, MoveFlag::PromotionBishop);
            moves.emplace_back(start, i, MoveFlag::PromotionKnight);
        }
    }

    // En Passant
    const int8_t ep = board.getEnPassantTargetSquare();
    if (ep != -1)
        [[unlikely]]
    {
        Bitboard enPassantPawns = pawns & (bitboards::withSquare(ep + 9 * direction) | bitboards::withSquare(
                                                                                           ep + 7 * direction));
        while (enPassantPawns != 0)
        {
            const Square i = bitboards::popMSB(enPassantPawns);
            if ((bitboards::withSquare(i) & bitboards::FILE_A) != 0 && (bitboards::withSquare(ep) & bitboards::FILE_H) != 0)
            {
                continue;
            }
            if ((bitboards::withSquare(i) & bitboards::FILE_H) != 0 &&
                (bitboards::withSquare(ep) & bitboards::FILE_A) != 0)
            {
                continue;
            }
            /*
            First, check whether we are in check after en passant because there are edge cases with the pin
            detection. This is not particularly efficient and there are better ways of doing this, but en passant
            is rare so there shouldn't be a significant performance impact. (TODO: Improve)
            */
            board.makeMove(Move{i, static_cast<Square>(ep), MoveFlag::EnPassant});
            const bool enPassantPossible = !board.isSideInCheck(side);
            board.unmakeMove();
            if (!enPassantPossible)
            {
                continue;
            }

            moves.emplace_back(i, ep, MoveFlag::EnPassant);
        }
    }
}

void generateKnightMoves(MoveList &moves, const Board &board, Bitboard checkResolutions)
{
    const PieceColor side = board.sideToMove;
    Bitboard knights = board.bitboards[Piece{PieceKind::KNIGHT, side}.index()];

    while (knights != 0)
    {
        const Square i = bitboards::popMSB(knights);
        Bitboard attackingSquares = knightAttackingSquares[i];

        // Ignore squares that are occupied by friendly pieces
        attackingSquares &= ~board.getPieces(side);
        attackingSquares &= pinLines[i] & checkResolutions;
        while (attackingSquares != 0)
        {
            const Square end = bitboards::popMSB(attackingSquares);
            moves.emplace_back(i, end, MoveFlag::None);
        }
    }
}

void generateBishopMoves(MoveList &moves, const Board &board, Bitboard checkResolutions)
{
    const PieceColor side = board.sideToMove;
    Bitboard bishops = board.bitboards[Piece{PieceKind::BISHOP, side}.index()];

    while (bishops != 0)
    {
        const Square i = bitboards::popMSB(bishops);

        const Bitboard blockers = board.getPieces() & BISHOP_BLOCKER_MASKS[i];
        Bitboard attackingSquares = BISHOP_ATTACKING_SQUARES[i][blockers * BISHOP_MAGICS[i] >> BISHOP_SHIFTS[i]];

        // Ignore squares that are occupied by friendly pieces
        attackingSquares &= ~board.getPieces(side);
        attackingSquares &= pinLines[i] & checkResolutions;
        while (attackingSquares != 0)
        {
            const Square end = bitboards::popMSB(attackingSquares);
            moves.emplace_back(i, end, MoveFlag::None);
        }
    }
}

void generateRookMoves(MoveList &moves, const Board &board, Bitboard checkResolutions)
{
    const PieceColor side = board.sideToMove;
    Bitboard rooks = board.bitboards[Piece{PieceKind::ROOK, side}.index()];

    while (rooks != 0)
    {
        const Square i = bitboards::popMSB(rooks);

        const Bitboard blockers = board.getPieces() & ROOK_BLOCKER_MASKS[i];
        Bitboard attackingSquares = ROOK_ATTACKING_SQUARES[i][blockers * ROOK_MAGICS[i] >> ROOK_SHIFTS[i]];

        // Ignore squares that are occupied by friendly pieces
        attackingSquares &= ~board.getPieces(side);
        attackingSquares &= pinLines[i] & checkResolutions;
        while (attackingSquares != 0)
        {
            const Square end = bitboards::popMSB(attackingSquares);
            moves.emplace_back(i, end, MoveFlag::None);
        }
    }
}

void generateQueenMoves(MoveList &moves, const Board &board, Bitboard checkResolutions)
{
    const PieceColor side = board.sideToMove;
    Bitboard queens = board.bitboards[Piece{PieceKind::QUEEN, side}.index()];

    while (queens != 0)
    {
        const Square i = bitboards::popMSB(queens);

        const Bitboard horizontalBlockers = board.getPieces() & ROOK_BLOCKER_MASKS[i];
        const Bitboard diagonalBlockers = board.getPieces() & BISHOP_BLOCKER_MASKS[i];
        const Bitboard horizontalSquares = ROOK_ATTACKING_SQUARES[i][(horizontalBlockers * ROOK_MAGICS[i]) >> ROOK_SHIFTS[i]];
        const Bitboard diagonalSquares = BISHOP_ATTACKING_SQUARES[i][(diagonalBlockers * BISHOP_MAGICS[i]) >> BISHOP_SHIFTS[i]];
        Bitboard attackingSquares = horizontalSquares | diagonalSquares;

        // Ignore squares that are occupied by friendly pieces
        attackingSquares &= ~board.getPieces(side);
        attackingSquares &= pinLines[i] & checkResolutions;
        while (attackingSquares != 0)
        {
            const Square end = bitboards::popMSB(attackingSquares);
            moves.emplace_back(i, end, MoveFlag::None);
        }
    }
}

void generateKingMoves(MoveList &moves, const Board &board)
{
    using enum PieceKind;
    const PieceColor side = board.sideToMove;
    const Bitboard king = board.bitboards[Piece{KING, side}.index()];
    const Square i = bitboards::getMSB(king);
    Bitboard attackingSquares = kingAttackingSquares[i];
    attackingSquares &= ~board.getPieces(side);

    Bitboard opponentAttackingSquares = board.getAttackingSquares(oppositeColor(side));
    // Generate check evasions when the king moves away from a sliding piece along its attacking diagonal
    // This is done by generating the attacking squares for sliding pieces as if the king wasn't there
    const Bitboard allPiecesWithoutKing = board.getPieces() & ~king;
    const Bitboard enemyBishops = board.bitboards[Piece{BISHOP, oppositeColor(side)}.index()];
    const Bitboard enemyRooks = board.bitboards[Piece{ROOK, oppositeColor(side)}.index()];
    const Bitboard enemyQueens = board.bitboards[Piece{QUEEN, oppositeColor(side)}.index()];
    opponentAttackingSquares |= getPieceAttackingSquares<BISHOP>(allPiecesWithoutKing, enemyBishops);
    opponentAttackingSquares |= getPieceAttackingSquares<ROOK>(allPiecesWithoutKing, enemyRooks);
    opponentAttackingSquares |= getPieceAttackingSquares<QUEEN>(allPiecesWithoutKing, enemyQueens);

    // Prevent the king from moving into check
    attackingSquares &= ~opponentAttackingSquares;

    while (attackingSquares != 0)
    {
        const Square targetSquare = bitboards::popMSB(attackingSquares);
        moves.emplace_back(i, targetSquare, MoveFlag::None);
    }

    // Castling
    if (!board.isSideInCheck(side))
    {
        if (side == WHITE)
        {
            if (board.canWhiteShortCastle() && (opponentAttackingSquares & bitboards::withSquare(i + 1)) == 0 && (opponentAttackingSquares & bitboards::withSquare(i + 2)) == 0 && board.isSquareEmpty(i + 1) && board.isSquareEmpty(i + 2))
            {
                moves.emplace_back(i, i + 2, MoveFlag::ShortCastling);
            }
            if (board.canWhiteLongCastle() && (opponentAttackingSquares & bitboards::withSquare(i - 1)) == 0 && (opponentAttackingSquares & bitboards::withSquare(i - 2)) == 0 && board.isSquareEmpty(i - 1) && board.isSquareEmpty(i - 2) && board.isSquareEmpty(i - 3))
            {
                moves.emplace_back(i, i - 2, MoveFlag::LongCastling);
            }
        }
        else
        {
            if (board.canBlackShortCastle() && (opponentAttackingSquares & bitboards::withSquare(i + 1)) == 0 && (opponentAttackingSquares & bitboards::withSquare(i + 2)) == 0 && board.isSquareEmpty(i + 1) && board.isSquareEmpty(i + 2))
            {
                moves.emplace_back(i, i + 2, MoveFlag::ShortCastling);
            }
            if (board.canBlackLongCastle() && (opponentAttackingSquares & bitboards::withSquare(i - 1)) == 0 && (opponentAttackingSquares & bitboards::withSquare(i - 2)) == 0 && board.isSquareEmpty(i - 1) && board.isSquareEmpty(i - 2) && board.isSquareEmpty(i - 3))
            {
                moves.emplace_back(i, i - 2, MoveFlag::LongCastling);
            }
        }
    }
}

MoveList generateLegalMoves(Board &board)
{
    MoveList moves;

    const PieceColor sideToMove = board.sideToMove;
    // Squares to which a piece other than the king can move to block a check
    computePinLinesAndSlidingCheckers(board, sideToMove);
    const Bitboard checkResolutions = board.isSideInCheck(sideToMove)
                                          ? checkResolutionSquares(board)
                                          : bitboards::ALL_SQUARES;

    generatePawnMoves(moves, board, checkResolutions);
    generateKnightMoves(moves, board, checkResolutions);
    generateBishopMoves(moves, board, checkResolutions);
    generateRookMoves(moves, board, checkResolutions);
    generateQueenMoves(moves, board, checkResolutions);
    generateKingMoves(moves, board);

    return moves;
}

Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side)
{
    const Bitboard leftCaptures = side == WHITE
                                      ? (pawns & ~bitboards::FILE_A) << 9
                                      : (pawns & ~bitboards::FILE_A) >> 7;
    const Bitboard rightCaptures = side == WHITE
                                       ? (pawns & ~bitboards::FILE_H) << 7
                                       : (pawns & ~bitboards::FILE_H) >> 9;
    return leftCaptures | rightCaptures;
}

std::array<Bitboard, 64> getBishopBlockerMasks()
{
    return BISHOP_BLOCKER_MASKS;
}

std::array<Bitboard, 64> getRookBlockerMasks()
{
    return ROOK_BLOCKER_MASKS;
}

template <PieceKind Kind>
Bitboard getPieceAttackingSquares(Bitboard allPieces, Bitboard pieces)
{
    using enum PieceKind;
    Bitboard squares = 0;
    while (pieces != 0)
    {
        const Square index = bitboards::popMSB(pieces);
        if constexpr (Kind == KNIGHT)
        {
            squares |= knightAttackingSquares[index];
        }
        else if constexpr (Kind == BISHOP)
        {
            const Bitboard blockers = allPieces & BISHOP_BLOCKER_MASKS[index];
            squares |= BISHOP_ATTACKING_SQUARES[index][blockers * BISHOP_MAGICS[index] >> BISHOP_SHIFTS[index]];
        }
        else if constexpr (Kind == ROOK)
        {
            const Bitboard blockers = allPieces & ROOK_BLOCKER_MASKS[index];
            squares |= ROOK_ATTACKING_SQUARES[index][blockers * ROOK_MAGICS[index] >> ROOK_SHIFTS[index]];
        }
        else if constexpr (Kind == QUEEN)
        {
            const Bitboard horizontalBlockers = allPieces & ROOK_BLOCKER_MASKS[index];
            const Bitboard diagonalBlockers = allPieces & BISHOP_BLOCKER_MASKS[index];
            const Bitboard horizontalSquares = ROOK_ATTACKING_SQUARES[index][(horizontalBlockers * ROOK_MAGICS[index]) >> ROOK_SHIFTS[index]];
            const Bitboard diagonalSquares = BISHOP_ATTACKING_SQUARES[index][(diagonalBlockers * BISHOP_MAGICS[index]) >> BISHOP_SHIFTS[index]];
            squares |= horizontalSquares | diagonalSquares;
        }
        else if constexpr (Kind == KING)
        {
            squares |= kingAttackingSquares[index];
        }
    }
    return squares;
}

template Bitboard getPieceAttackingSquares<PieceKind::KNIGHT>(Bitboard allPieces, Bitboard pieces);
template Bitboard getPieceAttackingSquares<PieceKind::BISHOP>(Bitboard allPieces, Bitboard pieces);
template Bitboard getPieceAttackingSquares<PieceKind::ROOK>(Bitboard allPieces, Bitboard pieces);
template Bitboard getPieceAttackingSquares<PieceKind::QUEEN>(Bitboard allPieces, Bitboard pieces);
template Bitboard getPieceAttackingSquares<PieceKind::KING>(Bitboard allPieces, Bitboard pieces);
} // namespace movegen
