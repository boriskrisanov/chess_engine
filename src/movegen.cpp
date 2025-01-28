#include "movegen.hpp"

#include <bit>
#include <iostream>

#include "bitboards.hpp"

using std::vector, std::array;
using enum PieceColor;

namespace movegen
{
    struct EdgeDistance
    {
        uint8_t left;
        uint8_t right;
        uint8_t top;
        uint8_t bottom;
        uint8_t topLeft;
        uint8_t topRight;
        uint8_t bottomLeft;
        uint8_t bottomRight;
    };

    enum class Direction : int8_t
    {
        UP = -8,
        DOWN = 8,
        LEFT = -1,
        RIGHT = 1,
        UP_LEFT = UP + LEFT,
        UP_RIGHT = UP + RIGHT,
        DOWN_LEFT = DOWN + LEFT,
        DOWN_RIGHT = DOWN + RIGHT
    };

    constexpr array<EdgeDistance, 64> edgeDistances = []() consteval
    {
        array<EdgeDistance, 64> distances{};

        for (int i = 0; i < 64; i++)
        {
            const Square rank = i / 8 + 1;
            auto& [left, right, top, bottom, topLeft, topRight, bottomLeft, bottomRight] = distances[i];

            left = 8 - (rank * 8 - i);
            right = rank * 8 - i - 1;
            top = i / 8;
            bottom = (63 - i) / 8;
            topLeft = std::min(top, left);
            topRight = std::min(top, right);
            bottomLeft = std::min(bottom, left);
            bottomRight = std::min(bottom, right);
        }

        return distances;
    }();

    constexpr uint8_t edgeDistanceInDirection(Square square, Direction direction)
    {
        switch (direction)
        {
        case Direction::UP:
            return edgeDistances[square].top;
        case Direction::DOWN:
            return edgeDistances[square].bottom;
        case Direction::LEFT:
            return edgeDistances[square].left;
        case Direction::RIGHT:
            return edgeDistances[square].right;
        case Direction::UP_LEFT:
            return edgeDistances[square].topLeft;
        case Direction::UP_RIGHT:
            return edgeDistances[square].topRight;
        case Direction::DOWN_LEFT:
            return edgeDistances[square].bottomLeft;
        case Direction::DOWN_RIGHT:
            return edgeDistances[square].bottomRight;
        }
        // Impossible since the switch handles all cases. This is here just to get rid of the compiler warning.
        return 0;
    }

    constexpr Bitboard rayAttackingSquares(Bitboard blockers, Square position, const vector<Direction>& directions)
    {
        Bitboard attackingSquares = 0;

        for (Direction direction : directions)
        {
            for (int i = 0; i < edgeDistanceInDirection(position, direction); i++)
            {
                int targetSquare = position + static_cast<int8_t>(direction) * (i + 1);
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
            if (edgeDistance.left >= 2 && edgeDistance.top >= 1)
            {
                squares |= bitboards::withSquare(i - 8 - 2);
            }
            if (edgeDistance.left >= 1 && edgeDistance.top >= 2)
            {
                squares |= bitboards::withSquare(i - 8 * 2 - 1);
            }
            if (edgeDistance.right >= 1 && edgeDistance.top >= 2)
            {
                squares |= bitboards::withSquare(i - 8 * 2 + 1);
            }
            if (edgeDistance.left >= 2 && edgeDistance.bottom >= 1)
            {
                squares |= bitboards::withSquare(i - 2 + 8);
            }
            if (edgeDistance.right >= 2 && edgeDistance.bottom >= 1)
            {
                squares |= bitboards::withSquare(i + 2 + 8);
            }
            if (edgeDistance.left >= 1 && edgeDistance.bottom >= 2)
            {
                squares |= bitboards::withSquare(i + 8 * 2 - 1);
            }
            if (edgeDistance.right >= 1 && edgeDistance.bottom >= 2)
            {
                squares |= bitboards::withSquare(i + 8 * 2 + 1);
            }
            if (edgeDistance.right >= 2 && edgeDistance.top >= 1)
            {
                squares |= bitboards::withSquare(i - 8 + 2);
            }
            bitboards[i] = squares;
        }
        return bitboards;
    }();

    array<Bitboard, 64> whitePawnAttackingSquares = []()
    {
        array<Bitboard, 64> bitboards{};
        for (Square i = 0; i < 64; i++)
        {
            Bitboard squares = 0;
            const auto edgeDistance = edgeDistances[i];
            if (edgeDistance.top > 0 && edgeDistance.right > 0)
            {
                squares |= bitboards::withSquare(i - 8 + 1);
            }
            if (edgeDistance.top > 0 && edgeDistance.left > 0)
            {
                squares |= bitboards::withSquare(i - 8 - 1);
            }
            bitboards[i] = squares;
        }
        return bitboards;
    }();

    array<Bitboard, 64> blackPawnAttackingSquares = []()
    {
        array<Bitboard, 64> bitboards{};
        for (Square i = 0; i < 64; i++)
        {
            Bitboard squares = 0;
            const auto edgeDistance = edgeDistances[i];
            if (edgeDistance.bottom > 0 && edgeDistance.right > 0)
            {
                squares |= bitboards::withSquare(i + 8 + 1);
            }
            if (edgeDistance.bottom > 0 && edgeDistance.left > 0)
            {
                squares |= bitboards::withSquare(i + 8 - 1);
            }
            bitboards[i] = squares;
        }
        return bitboards;
    }();

    constexpr array<Bitboard, 64> kingAttackingSquares = []() consteval
    {
        array<Bitboard, 64> bitboards{};
        for (Square i = 0; i < 64; i++)
        {
            Bitboard squares = 0;
            const auto edgeDistance = edgeDistances[i];
            if (edgeDistance.left >= 1)
            {
                squares |= bitboards::withSquare(i - 1);
            }
            if (edgeDistance.right >= 1)
            {
                squares |= bitboards::withSquare(i + 1);
            }
            if (edgeDistance.top >= 1)
            {
                squares |= bitboards::withSquare(i - 8);
            }
            if (edgeDistance.bottom >= 1)
            {
                squares |= bitboards::withSquare(i + 8);
            }
            if (edgeDistance.left >= 1 && edgeDistance.top >= 1)
            {
                squares |= bitboards::withSquare(i - 8 - 1);
            }
            if (edgeDistance.right >= 1 && edgeDistance.top >= 1)
            {
                squares |= bitboards::withSquare(i - 8 + 1);
            }
            if (edgeDistance.left >= 1 && edgeDistance.bottom >= 1)
            {
                squares |= bitboards::withSquare(i + 8 - 1);
            }
            if (edgeDistance.right >= 1 && edgeDistance.bottom >= 1)
            {
                squares |= bitboards::withSquare(i + 8 + 1);
            }
            bitboards[i] = squares;
        }
        return bitboards;
    }();


    struct KingRay
    {
        Bitboard bitboard;
        Direction direction;
    };

    constexpr array<array<KingRay, 8>, 64> precomputeKingRays()
    {
        array<array<KingRay, 8>, 64> rays{};

        for (Square i = 0; i < 64; i++)
        {
            rays[i][0] = {rayAttackingSquares(0, i, {Direction::UP}), Direction::UP};
            rays[i][1] = {rayAttackingSquares(0, i, {Direction::DOWN}), Direction::DOWN};
            rays[i][2] = {rayAttackingSquares(0, i, {Direction::LEFT}), Direction::LEFT};
            rays[i][3] = {rayAttackingSquares(0, i, {Direction::RIGHT}), Direction::RIGHT};
            rays[i][4] = {rayAttackingSquares(0, i, {Direction::UP_LEFT}), Direction::UP_LEFT};
            rays[i][5] = {rayAttackingSquares(0, i, {Direction::UP_RIGHT}), Direction::UP_RIGHT};
            rays[i][6] = {rayAttackingSquares(0, i, {Direction::DOWN_LEFT}), Direction::DOWN_LEFT};
            rays[i][7] = {rayAttackingSquares(0, i, {Direction::DOWN_RIGHT}), Direction::DOWN_RIGHT};
        }

        return rays;
    }

    const auto KING_RAYS = precomputeKingRays();

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
        0xb7c8ffffbdf8ed79, 0x7cccb4acac99a09a, 0x277f8a1f457fa352, 0x7e7d01513baf5767, 0xea6fff8a18fecce7,
        0x5d24e354a272711, 0xcb734ff54bfdceab, 0xc796020f8482c023, 0xcd8c8f85cd8c7798, 0xeaa063aac121fd78,
        0xdc1e46605b34c09c, 0xcbacc491fc4f54bc, 0x8036e0e6d8f8d7b8, 0xd3b647d77960e7d8, 0x9b20d4fa1bc46876,
        0x44c4264f0b18de1e, 0x8855b001ac251d80, 0x9625d5292d2e3c8e, 0xdbda6f4a66e590a7, 0x829058c99069906d,
        0xc9c0b0ea9c5521fb, 0x4177cd4386a64fab, 0x324a8dbe2ff95405, 0x55cd15e172a8d76f, 0xfb64a8f2415d7821,
        0xe7e48fdaafbff944, 0xbbb74318d41d9980, 0x11ab8facd32cad62, 0x10fcc8bc23373750, 0x528b8b07f650b407,
        0xe2ec3ddbe240271, 0x658d05b962e98275, 0xf70541a9e66a28a3, 0x79336c523e22a894, 0xe0543017e7f2ea61,
        0x626d5cde515429f3, 0xda285c3eb049a381, 0xb33e026abed080c8, 0x4fd05955da71f2bd, 0x6f5e84d217ad0bd7,
        0x96cd81400f2a7f68, 0x815be01fdbcb6d01, 0x66d6a657bfde74ac, 0xed07915ff915e160, 0x4267b33c3ccf4512,
        0xb945f45e60bc88c0, 0x6f25882bfdacac61, 0xc16006db41b8fc7e, 0x327dfffdbe7ae3aa, 0x7cf6fa0a0d05f415,
        0xc480c82b51c4a8df, 0xf43028053a4e4b4, 0x5475cff715cffbd0, 0xdea9695deb61b438, 0xac8aea22a7dbf996,
        0x73aecf15f4cd6390, 0xd6f50be59bf640b1, 0xa587df828f4368ab, 0x3581646cb6083d6b, 0xe4ded3bf94deb829,
        0x1878781a0a5f7d3a, 0x7a1ca6b38e4a76a1, 0x3322c373d920ddc6, 0x62ca191005858111
    };
    constexpr array<uint8_t, 64> ROOK_SHIFTS{
        50, 50, 50, 50, 51, 50, 50, 49, 51, 52, 51, 51, 51, 52, 52, 51, 51, 52, 52, 52, 51, 51, 52, 50, 51, 52, 51, 51,
        52,
        51, 52, 50, 51, 52, 52, 51, 51, 51, 52, 51, 51, 52, 51, 52, 51, 51, 52, 51, 51, 52, 51, 51, 52, 52, 52, 50, 49,
        50,
        50, 50, 50, 50, 50, 49
    };

    constexpr array<uint64_t, 64> BISHOP_MAGICS = {
        0xb8d001f098f81e00, 0x608526004064090, 0x584f1948600c9c91, 0xe2333ab7e2602083, 0xbb8eb4dc10882089,
        0x9aa25ead2c633000, 0x6e1bbba2880e8d21, 0xe361039861b637db, 0x464ecb40f41fe041, 0xe0c0f80f83c7830c,
        0x27e4caa0650d407f, 0x421b379212440abe, 0x23bcf95910410bce, 0xca7e6ba3a5100445, 0x869e968d7420139d,
        0xee4020cc9082543, 0x9b790e4c8a02b092, 0xd1eb1b0b0709a40b, 0x399de3efefb62600, 0x98ee6e703b6d575,
        0xe96c65008088e041, 0x399928d647fdeffb, 0xfe5924841912a45c, 0xfe14f07af8e50e04, 0x50349f0231d66c00,
        0xcf50a44c8eccf800, 0x320f04daf0528793, 0x16f2bedbffd3bddc, 0xb0987fefca7fbfd2, 0x5e244b495bad4658,
        0xcbfe038e2de72e2a, 0x1a1887c884c4e03b, 0xaf91c17679c0e63f, 0x37aa2398eb380684, 0x459c2357a2543de8,
        0x538185e1d430c2fa, 0x935571681f6fdbf7, 0xc25f97052e844918, 0x51a3a00c9757160b, 0x1f84963ba6f603c4,
        0x7fcb81d3861900f, 0x3e9c82fc08908805, 0x39938dcad7a938e1, 0xd0692149024012d8, 0x81e0cf3e5f758447,
        0xf763b7f04f3b4f05, 0xe96010e61600ce68, 0x28d424ea68102500, 0x3632c7bfbbff7760, 0x18ef575d72945d9d,
        0x15994bdf7befd422, 0xfcef1e05ee55acd1, 0xd7a4e6066ac05c8e, 0x81589a1a23410129, 0x8640b904cc7c8083,
        0xe774703dd07f8f7a, 0x6b02be82acde54f5, 0xbdf88210810427d1, 0x7de5c389dc68f251, 0xb1d8820280d514d7,
        0xf348042a41ee4af1, 0x19c01e38b70e474, 0x47632631826a015f, 0xe226625000cede4c
    };
    constexpr array<uint8_t, 64> BISHOP_SHIFTS = {
        57, 58, 58, 58, 58, 58, 58, 56, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 55, 55, 56, 55, 58, 58, 58, 58, 55, 52,
        53,
        55, 58, 58, 58, 58, 55, 53, 53, 55, 58, 58, 58, 58, 55, 56, 55, 55, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 56,
        58,
        58, 58, 58, 58, 58, 56
    };


    array<Bitboard, 64> ROOK_BLOCKER_MASKS = []()
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

    array<Bitboard, 64> BISHOP_BLOCKER_MASKS = []()
    {
        array<Bitboard, 64> masks{};
        for (int bishopIndex = 0; bishopIndex < 64; bishopIndex++)
        {
            const array directions{
                Direction::UP_LEFT, Direction::UP_RIGHT, Direction::DOWN_LEFT, Direction::DOWN_RIGHT
            };
            for (Direction direction : directions)
            {
                for (int i = 0; i < edgeDistanceInDirection(bishopIndex, direction); i++)
                {
                    Square targetSquare = bishopIndex + static_cast<uint8_t>(direction) * (i + 1);
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
            for (Bitboard blockerPositions : possibleBlockerPositions(ROOK_BLOCKER_MASKS[i]))
            {
                const uint32_t index = blockerPositions * ROOK_MAGICS[i] >> ROOK_SHIFTS[i];
                attackingSquares[i][index] = rayAttackingSquares(blockerPositions, i, {
                                                                     Direction::UP, Direction::DOWN, Direction::LEFT,
                                                                     Direction::RIGHT
                                                                 });
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
            for (Bitboard blockerPositions : possibleBlockerPositions(BISHOP_BLOCKER_MASKS[i]))
            {
                const uint32_t index = blockerPositions * BISHOP_MAGICS[i] >> BISHOP_SHIFTS[i];
                attackingSquares[i][index] = rayAttackingSquares(blockerPositions, i, {
                                                                     Direction::UP_LEFT, Direction::UP_RIGHT,
                                                                     Direction::DOWN_LEFT,
                                                                     Direction::DOWN_RIGHT
                                                                 });
            }
        }

        return attackingSquares;
    }

    // TODO: Use array instead of vector (array of pointers to arrays of different lengths)
    const array<vector<Bitboard>, 64> ROOK_ATTACKING_SQUARES = computeRookAttackingSquares();
    const array<vector<Bitboard>, 64> BISHOP_ATTACKING_SQUARES = computeBishopAttackingSquares();

    Bitboard generateAttackingSquares(Piece piece, Square position, const Board& board)
    {
        // TODO: Use diagonal edge distances
        Bitboard squares = 0;
        switch (piece.kind())
        {
        case PieceKind::PAWN:
            squares = piece.color() == WHITE
                          ? whitePawnAttackingSquares[position]
                          : blackPawnAttackingSquares[position];
            break;
        case PieceKind::KNIGHT:
            squares = knightAttackingSquares[position];
            break;
        case PieceKind::BISHOP:
            {
                const Bitboard blockers = board.getPieces() & BISHOP_BLOCKER_MASKS[position];
                squares = BISHOP_ATTACKING_SQUARES[position][blockers * BISHOP_MAGICS[position] >> BISHOP_SHIFTS[
                    position]];
            }
            break;
        case PieceKind::ROOK:
            {
                const Bitboard blockers = board.getPieces() & ROOK_BLOCKER_MASKS[position];
                squares = ROOK_ATTACKING_SQUARES[position][blockers * ROOK_MAGICS[position] >> ROOK_SHIFTS[position]];
            }
            break;
        case PieceKind::QUEEN:
            {
                const Bitboard horizontalBlockers = board.getPieces() & ROOK_BLOCKER_MASKS[position];
                const Bitboard diagonalBlockers = board.getPieces() & BISHOP_BLOCKER_MASKS[position];
                const Bitboard horizontalSquares = ROOK_ATTACKING_SQUARES[position][(horizontalBlockers * ROOK_MAGICS[
                    position]) >> ROOK_SHIFTS[position]];
                const Bitboard diagonalSquares = BISHOP_ATTACKING_SQUARES[position][(diagonalBlockers * BISHOP_MAGICS[
                    position]) >> BISHOP_SHIFTS[position]];
                squares = horizontalSquares | diagonalSquares;
            }
            break;
        case PieceKind::KING:
            squares = kingAttackingSquares[position];
            break;
        default:
            break;
        }

        return squares;
    }

    // Inefficient, fine for now
    vector<Direction> rayCheckDirections;
    bool isRayCheck;

    Bitboard checkResolutionSquares(const Board& board)
    {
        using enum Direction;

        Bitboard squares = 0;
        Square kingPos = board.sideToMove == WHITE ? board.whiteKingPosition : board.blackKingPosition;

        // Sliding pieces
        // TODO: Use bitboards
        uint8_t numSlidingCheckDirections = 0;
        for (const auto [rayBitboard, direction] : KING_RAYS[kingPos])
        {
            if (board.getSlidingPieces(oppositeColor(board.sideToMove)) & rayBitboard == 0)
            {
                // No enemy sliding pieces along this ray, so the king can't be in check from this direction
                continue;
            }
            Bitboard raySquares = 0;
            for (Square i = 0; i < edgeDistanceInDirection(kingPos, direction); i++)
            {
                Square targetSquare = kingPos + static_cast<int>(direction) * (i + 1);
                if (board[targetSquare].isNone())
                {
                    raySquares |= bitboards::withSquare(targetSquare);
                }
                else if (((board[targetSquare].kind() == PieceKind::ROOK && (direction == UP || direction == DOWN ||
                    direction
                    == LEFT || direction == RIGHT)) || (board[targetSquare].kind() == PieceKind::BISHOP && (direction ==
                    UP_LEFT || direction == UP_RIGHT || direction == DOWN_LEFT || direction == DOWN_RIGHT)) || board[
                    targetSquare].kind() == PieceKind::QUEEN) && board[targetSquare].color() != board.sideToMove)
                {
                    // In check from this direction
                    squares |= raySquares;
                    // Attacking piece can be captured
                    squares |= bitboards::withSquare(targetSquare);
                    numSlidingCheckDirections++;

                    isRayCheck = true;
                    rayCheckDirections.push_back(direction);

                    break;
                }
                else if (!board[targetSquare].isNone())
                {
                    // Piece is in the way, not in check from this direction
                    break;
                }
            }
        }

        bool checkFromNonSlidingPiece = false;

        // Pawns
        {
            int p1;
            int p2;
            bool edgeDistanceRequirement1, edgeDistanceRequirement2;
            if (board.sideToMove == WHITE)
            {
                p1 = kingPos + static_cast<int>(UP_LEFT);
                p2 = kingPos + static_cast<int>(UP_RIGHT);
                edgeDistanceRequirement1 = edgeDistanceInDirection(kingPos, UP_LEFT) > 0;
                edgeDistanceRequirement2 = edgeDistanceInDirection(kingPos, UP_RIGHT) > 0;
            }
            else
            {
                p1 = kingPos + static_cast<int>(DOWN_LEFT);
                p2 = kingPos + static_cast<int>(DOWN_RIGHT);
                edgeDistanceRequirement1 = edgeDistanceInDirection(kingPos, DOWN_LEFT) > 0;
                edgeDistanceRequirement2 = edgeDistanceInDirection(kingPos, DOWN_RIGHT) > 0;
            }
            if (edgeDistanceRequirement1 && board[p1].kind() == PieceKind::PAWN && board[p1].color() == oppositeColor(
                board.sideToMove))
            {
                squares |= bitboards::withSquare(p1);
                checkFromNonSlidingPiece = true;
            }
            else if (edgeDistanceRequirement2 && board[p2].kind() == PieceKind::PAWN && board[p2].color() ==
                oppositeColor(
                    board.sideToMove))
            {
                squares |= bitboards::withSquare(p2);
                checkFromNonSlidingPiece = true;
            }
        }

        // Knights
        {
            Bitboard knightPositions = 0;
            EdgeDistance edgeDistance = edgeDistances[kingPos];

            if (edgeDistance.left >= 1 && edgeDistance.top >= 2)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(LEFT) + static_cast<int>(UP) * 2);
            }
            if (edgeDistance.right >= 1 && edgeDistance.top >= 2)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(RIGHT) + static_cast<int>(UP) * 2);
            }
            if (edgeDistance.left >= 1 && edgeDistance.bottom >= 2)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(LEFT) + static_cast<int>(DOWN) * 2);
            }
            if (edgeDistance.right >= 1 && edgeDistance.bottom >= 2)
            {
                knightPositions |=
                    bitboards::withSquare(kingPos + static_cast<int>(RIGHT) + static_cast<int>(DOWN) * 2);
            }
            if (edgeDistance.left >= 2 && edgeDistance.top >= 1)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(LEFT) * 2 + static_cast<int>(UP));
            }
            if (edgeDistance.left >= 2 && edgeDistance.bottom >= 1)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(LEFT) * 2 + static_cast<int>(DOWN));
            }
            if (edgeDistance.right >= 2 && edgeDistance.top >= 1)
            {
                knightPositions |= bitboards::withSquare(kingPos + static_cast<int>(RIGHT) * 2 + static_cast<int>(UP));
            }
            if (edgeDistance.right >= 2 && edgeDistance.bottom >= 1)
            {
                knightPositions |=
                    bitboards::withSquare(kingPos + static_cast<int>(RIGHT) * 2 + static_cast<int>(DOWN));
            }

            for (Square position : bitboards::squaresOf(knightPositions))
            {
                if (board[position].kind() == PieceKind::KNIGHT && board[position].color() == oppositeColor(
                    board.sideToMove))
                {
                    squares |= bitboards::withSquare(position);
                    checkFromNonSlidingPiece = true;
                    // There cannot be a check from 2 knights at once, so there is no need to check other possible knight positions
                    break;
                }
            }
        }

        if ((numSlidingCheckDirections > 0 && checkFromNonSlidingPiece) || numSlidingCheckDirections > 1)
        [[unlikely]]
        {
            // Discovered double check, there are no resolution squares because it cannot be blocked and both pieces
            // cannot be captured in one move.
            squares = 0;
        }

        return squares;
    }

    Bitboard pinnedPieces;
    // Not the most efficient, but fine for now
    array<Bitboard, 64> pinLines{};

    void computePinLines(const Board& board, PieceColor side)
    {
        using enum Direction;
        pinnedPieces = 0;
        pinLines = array<Bitboard, 64>{};

        int kingPosition = side == WHITE ? board.whiteKingPosition : board.blackKingPosition;

        for (const auto [rayBitboard, direction] : KING_RAYS[kingPosition])
        {
            if ((rayBitboard & board.getSlidingPieces(oppositeColor(side))) == 0)
            {
                // No enemy sliding pieces on this diagonal, so no pieces on it can be pinned
                continue;
            }

            Piece lastFriendlyPieceSeen{};
            Square lastFriendlyPiecePos = 0;

            Bitboard pinSquares = 0;
            for (int i = 0; i < edgeDistanceInDirection(kingPosition, direction); i++)
            {
                const Square targetSquare = kingPosition + static_cast<int>(direction) * (i + 1);

                const bool kingCanBeAttackedByRook = direction == UP || direction == DOWN || direction == LEFT ||
                    direction
                    == RIGHT;
                const bool kingCanBeAttackedByBishop = direction == UP_LEFT || direction == UP_RIGHT || direction ==
                    DOWN_LEFT || direction == DOWN_RIGHT;
                const bool targetPieceCanAttackKing = board[targetSquare].kind() == PieceKind::QUEEN || (board[
                        targetSquare]
                    .
                    kind() == PieceKind::ROOK && kingCanBeAttackedByRook) || (board[targetSquare].kind() ==
                    PieceKind::BISHOP &&
                    kingCanBeAttackedByBishop);

                if (board[targetSquare].isNone())
                {
                    pinSquares |= bitboards::withSquare(targetSquare);
                    continue;
                }

                if (!targetPieceCanAttackKing && board[targetSquare].color() == oppositeColor(side) &&
                    lastFriendlyPieceSeen
                    .
                    isNone())
                {
                    // Not in check from this direction
                    break;
                }

                if (board[targetSquare].isSlidingPiece() && board[targetSquare].color() != side
                    && (((direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT) && (board[
                            targetSquare].kind() == PieceKind::ROOK || board[targetSquare].kind() == PieceKind::QUEEN))
                        || ((direction == UP_LEFT || direction == UP_RIGHT || direction == DOWN_LEFT || direction ==
                            DOWN_RIGHT) && (board[targetSquare].kind() == PieceKind::BISHOP || board[targetSquare].
                            kind() ==
                            PieceKind::QUEEN)))
                )
                {
                    if (lastFriendlyPieceSeen.isNone())
                    {
                        // King is in check from this direction
                        break;
                    }

                    pinSquares |= bitboards::withSquare(targetSquare);
                    pinLines[lastFriendlyPiecePos] = pinSquares;
                    pinnedPieces |= bitboards::withSquare(lastFriendlyPiecePos);
                    break;
                }
                if (!lastFriendlyPieceSeen.isNone() && !board[targetSquare].isNone())
                {
                    // There are more than 2 pieces in front of the king, therefore none of them are pinned
                    break;
                }
                if (board[targetSquare].color() == side)
                {
                    lastFriendlyPieceSeen = board[targetSquare];
                    lastFriendlyPiecePos = targetSquare;
                    pinSquares |= bitboards::withSquare(targetSquare);
                }
            }
        }
    }

    void generatePawnMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard pawns = board.bitboards[Piece{PieceKind::PAWN, side}.index()];
        const Bitboard emptySquares = ~board.getPieces();
        const Bitboard enemyPieces = board.getPieces(oppositeColor(side));
        const int direction = side == WHITE ? 1 : -1;


        const Bitboard doublePushTarget = side == WHITE ? bitboards::RANK_4 : bitboards::RANK_5;
        const Bitboard promotionRank = side == WHITE ? bitboards::RANK_1 : bitboards::RANK_8;
        Bitboard singlePushes = (side == WHITE ? pawns << 8 : pawns >> 8) & emptySquares;
        Bitboard doublePushes = (side == WHITE ? singlePushes << 8 : singlePushes >> 8) & emptySquares &
            doublePushTarget;
        Bitboard leftCaptures = (side == WHITE
                                     ? ((pawns & ~bitboards::FILE_A) << 9)
                                     : ((pawns & ~bitboards::FILE_A) >> 7)) & enemyPieces;
        Bitboard rightCaptures = (side == WHITE
                                      ? ((pawns & ~bitboards::FILE_H) << 7)
                                      : ((pawns & ~bitboards::FILE_H) >> 9)) & enemyPieces;

        Bitboard singlePushesWithPromotion = singlePushes & promotionRank;
        Bitboard leftCapturesWithPromotion = leftCaptures & promotionRank;
        Bitboard rightCapturesWithPromotion = rightCaptures & promotionRank;

        singlePushes &= ~promotionRank;
        leftCaptures &= ~promotionRank;
        rightCaptures &= ~promotionRank;

        // Pawn pushes
        while (singlePushes != 0)
        {
            Square i = bitboards::popMSB(singlePushes);
            const Square start = i + 8 * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::None);
            }
        }
        while (doublePushes != 0)
        {
            Square i = bitboards::popMSB(doublePushes);
            const Square start = i + 16 * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::None);
            }
        }

        // Captures
        while (leftCaptures != 0)
        {
            Square i = bitboards::popMSB(leftCaptures);
            const Square start = i + (side == WHITE ? 9 : 7) * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::None);
            }
        }
        while (rightCaptures != 0)
        {
            Square i = bitboards::popMSB(rightCaptures);
            const Square start = i + (side == WHITE ? 7 : 9) * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::None);
            }
        }

        // Promotion
        while (singlePushesWithPromotion != 0)
        {
            Square i = bitboards::popMSB(singlePushesWithPromotion);
            const Square start = i + 8 * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
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
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::PromotionQueen);
                moves.emplace_back(start, i, MoveFlag::PromotionRook);
                moves.emplace_back(start, i, MoveFlag::PromotionBishop);
                moves.emplace_back(start, i, MoveFlag::PromotionKnight);
            }
        }
        while (rightCapturesWithPromotion != 0)
        {
            Square i = bitboards::popMSB(rightCapturesWithPromotion);
            const Square start = i + (side == WHITE ? 7 : 9) * direction;
            const bool isPinned = (bitboards::withSquare(start) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[start] : bitboards::ALL_SQUARES;
            const Bitboard targetBitboard = bitboards::withSquare(i);
            if ((checkResolutions & targetBitboard) != 0 && (pinLine & targetBitboard) != 0)
            {
                moves.emplace_back(start, i, MoveFlag::PromotionQueen);
                moves.emplace_back(start, i, MoveFlag::PromotionRook);
                moves.emplace_back(start, i, MoveFlag::PromotionBishop);
                moves.emplace_back(start, i, MoveFlag::PromotionKnight);
            }
        }

        // En Passant
        const Square ep = board.getEnPassantTargetSquare();
        if (ep != -1)
        [[unlikely]]
        {
            Bitboard enPassantPawns = pawns & (bitboards::withSquare(ep + 9 * direction) | bitboards::withSquare(
                ep + 7 * direction));
            while (enPassantPawns != 0)
            {
                Square i = bitboards::popMSB(enPassantPawns);
                if ((bitboards::withSquare(i) & bitboards::FILE_A) != 0
                    && (bitboards::withSquare(ep) & bitboards::FILE_H) != 0)
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
                    break;
                }

                moves.emplace_back(i, ep, MoveFlag::EnPassant);
            }
        }
    }

    void generateKnightMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard knights = board.bitboards[Piece{PieceKind::KNIGHT, side}.index()];

        while (knights != 0)
        {
            const Square i = bitboards::popMSB(knights);
            Bitboard attackingSquares = knightAttackingSquares[i];
            const bool isPinned = (bitboards::withSquare(i) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[i] : bitboards::ALL_SQUARES;

            // Ignore squares that are occupied by friendly pieces
            attackingSquares &= ~board.getPieces(side);
            attackingSquares &= pinLine & checkResolutions;
            for (Square square : bitboards::squaresOf(attackingSquares))
            {
                moves.emplace_back(i, square, MoveFlag::None);
            }
        }
    }

    void generateBishopMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard bishops = board.bitboards[Piece{PieceKind::BISHOP, side}.index()];

        while (bishops != 0)
        {
            const Square i = bitboards::popMSB(bishops);

            const Bitboard blockers = board.getPieces() & BISHOP_BLOCKER_MASKS[i];
            Bitboard attackingSquares = BISHOP_ATTACKING_SQUARES[i][blockers * BISHOP_MAGICS[i] >> BISHOP_SHIFTS[i]];

            const bool isPinned = (bitboards::withSquare(i) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[i] : bitboards::ALL_SQUARES;

            // Ignore squares that are occupied by friendly pieces
            attackingSquares &= ~board.getPieces(side);
            attackingSquares &= pinLine & checkResolutions;
            for (Square square : bitboards::squaresOf(attackingSquares))
            {
                moves.emplace_back(i, square, MoveFlag::None);
            }
        }
    }

    void generateRookMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard rooks = board.bitboards[Piece{PieceKind::ROOK, side}.index()];

        while (rooks != 0)
        {
            const Square i = bitboards::popMSB(rooks);

            const Bitboard blockers = board.getPieces() & ROOK_BLOCKER_MASKS[i];
            Bitboard attackingSquares = ROOK_ATTACKING_SQUARES[i][blockers * ROOK_MAGICS[i] >> ROOK_SHIFTS[i]];

            const bool isPinned = (bitboards::withSquare(i) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[i] : bitboards::ALL_SQUARES;

            // Ignore squares that are occupied by friendly pieces
            attackingSquares &= ~board.getPieces(side);
            attackingSquares &= pinLine & checkResolutions;
            for (Square square : bitboards::squaresOf(attackingSquares))
            {
                moves.emplace_back(i, square, MoveFlag::None);
            }
        }
    }

    void generateQueenMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard queens = board.bitboards[Piece{PieceKind::QUEEN, side}.index()];

        while (queens != 0)
        {
            const Square i = bitboards::popMSB(queens);

            const Bitboard horizontalBlockers = board.getPieces() & ROOK_BLOCKER_MASKS[i];
            const Bitboard diagonalBlockers = board.getPieces() & BISHOP_BLOCKER_MASKS[i];
            const Bitboard horizontalSquares = ROOK_ATTACKING_SQUARES[i][(horizontalBlockers * ROOK_MAGICS[
                i]) >> ROOK_SHIFTS[i]];
            const Bitboard diagonalSquares = BISHOP_ATTACKING_SQUARES[i][(diagonalBlockers * BISHOP_MAGICS[
                i]) >> BISHOP_SHIFTS[i]];
            Bitboard attackingSquares = horizontalSquares | diagonalSquares;

            const bool isPinned = (bitboards::withSquare(i) & pinnedPieces) != 0;
            const Bitboard pinLine = isPinned ? pinLines[i] : bitboards::ALL_SQUARES;

            // Ignore squares that are occupied by friendly pieces
            attackingSquares &= ~board.getPieces(side);
            attackingSquares &= pinLine & checkResolutions;
            for (Square square : bitboards::squaresOf(attackingSquares))
            {
                moves.emplace_back(i, square, MoveFlag::None);
            }
        }
    }

    void generateKingMoves(std::vector<Move>& moves, Board board, Bitboard checkResolutions)
    {
        const PieceColor side = board.sideToMove;
        Bitboard king = board.bitboards[Piece{PieceKind::KING, side}.index()];
        Square i = bitboards::popMSB(king);
        Bitboard attackingSquares = kingAttackingSquares[i];
        attackingSquares &= ~board.getPieces(side);
        const bool isPinned = (bitboards::withSquare(i) & pinnedPieces) != 0;
        const Bitboard pinLine = isPinned ? pinLines[i] : bitboards::ALL_SQUARES;
        attackingSquares &= pinLine;

        const Bitboard opponentAttackingSquares = board.getAttackingSquares(oppositeColor(side));
        // Prevent the king from moving into check
        attackingSquares &= ~opponentAttackingSquares;

        for (Square targetSquare : bitboards::squaresOf(attackingSquares))
        {
            if (isRayCheck)
            {
                /*
                Check that the square the king is moving to is still not attacked after the move. This can happen
                when moving away from a sliding piece along its attack ray, as the square behind the king
                wouldn't be attacked since the attacking squares would only be recomputed after the move. For
                example, in "k7/8/8/2q5/8/4K3/8/8 w", Kf2 is generated as a legal move, even though the king
                would still be in check.
                */
                using enum Direction;
                using std::ranges::find;
                if ((find(rayCheckDirections, UP) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<
                            int>(DOWN))
                    || (find(rayCheckDirections, DOWN) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(UP))
                    || (find(rayCheckDirections, LEFT) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(RIGHT))
                    || (find(rayCheckDirections, RIGHT) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(LEFT))
                    || (find(rayCheckDirections, UP_LEFT) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(DOWN_RIGHT))
                    || (find(rayCheckDirections, UP_RIGHT) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(DOWN_LEFT))
                    || (find(rayCheckDirections, DOWN_LEFT) != rayCheckDirections.end() && targetSquare == i +
                        static_cast<int>(UP_RIGHT))
                    || (find(rayCheckDirections, DOWN_RIGHT) != rayCheckDirections.end() && targetSquare == i
                        +
                        static_cast<int>(UP_LEFT)))
                {
                    continue;
                }
            }

            moves.emplace_back(i, targetSquare, MoveFlag::None);
        }


        // Castling
        if (!board.isSideInCheck(side))
        {
            if (side == WHITE)
            {
                if (board.canWhiteShortCastle()
                    && (opponentAttackingSquares & bitboards::withSquare(i + 1)) == 0
                    && (opponentAttackingSquares & bitboards::withSquare(i + 2)) == 0
                    && board.isSquareEmpty(i + 1)
                    && board.isSquareEmpty(i + 2)
                )
                {
                    moves.emplace_back(i, i + 2, MoveFlag::ShortCastling);
                }
                if (board.canWhiteLongCastle()
                    && (opponentAttackingSquares & bitboards::withSquare(i - 1)) == 0
                    && (opponentAttackingSquares & bitboards::withSquare(i - 2)) == 0
                    && board.isSquareEmpty(i - 1)
                    && board.isSquareEmpty(i - 2)
                    && board.isSquareEmpty(i - 3)
                )
                {
                    moves.emplace_back(i, i - 2, MoveFlag::LongCastling);
                }
            }
            else
            {
                if (board.canBlackShortCastle()
                    && (opponentAttackingSquares & bitboards::withSquare(i + 1)) == 0
                    && (opponentAttackingSquares & bitboards::withSquare(i + 2)) == 0
                    && board.isSquareEmpty(i + 1)
                    && board.isSquareEmpty(i + 2)
                )
                {
                    moves.emplace_back(i, i + 2, MoveFlag::ShortCastling);
                }
                if (board.canBlackLongCastle()
                    && (opponentAttackingSquares & bitboards::withSquare(i - 1)) == 0
                    && (opponentAttackingSquares & bitboards::withSquare(i - 2)) == 0
                    && board.isSquareEmpty(i - 1)
                    && board.isSquareEmpty(i - 2)
                    && board.isSquareEmpty(i - 3)
                )
                {
                    moves.emplace_back(i, i - 2, MoveFlag::LongCastling);
                }
            }
        }
    }


    std::vector<Move> generateLegalMoves(Board& board)
    {
        vector<Move> moves;
        moves.reserve(255);

        // TODO: Improve architecture and minimise use of globals
        // Computed by checkResolutionSquares()
        isRayCheck = false;
        rayCheckDirections.clear();
        const PieceColor sideToMove = board.sideToMove;
        // Squares to which a piece other than the king can move to block a check
        const Bitboard checkResolutions = board.isSideInCheck(sideToMove)
                                              ? checkResolutionSquares(board)
                                              : bitboards::ALL_SQUARES;
        computePinLines(board, sideToMove);

        generatePawnMoves(moves, board, checkResolutions);
        generateKnightMoves(moves, board, checkResolutions);
        generateBishopMoves(moves, board, checkResolutions);
        generateRookMoves(moves, board, checkResolutions);
        generateQueenMoves(moves, board, checkResolutions);
        generateKingMoves(moves, board, checkResolutions);

        return moves;
    }

    Bitboard getPawnAttackingSquares(Bitboard pawns, PieceColor side)
    {
        Bitboard squares = 0;
        while (pawns != 0)
        {
            Square index = bitboards::popMSB(pawns);
            squares |= (side == WHITE ? whitePawnAttackingSquares[index] : blackPawnAttackingSquares[index]);
        }
        return squares;
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
                const Bitboard horizontalSquares = ROOK_ATTACKING_SQUARES[index][(horizontalBlockers * ROOK_MAGICS[
                    index]) >> ROOK_SHIFTS[index]];
                const Bitboard diagonalSquares = BISHOP_ATTACKING_SQUARES[index][(diagonalBlockers * BISHOP_MAGICS[
                    index]) >> BISHOP_SHIFTS[index]];
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
}
