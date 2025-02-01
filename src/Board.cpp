#include "Board.hpp"

#include <random>
#include <regex>

#include "movegen.hpp"
#include "utils.hpp"

using std::string;

using enum PieceKind;
using enum PieceColor;

void Board::loadFen(string fen)
{
    hashHistory = {};
    for (int i = 0; i < 64; i++)
    {
        board[i] = Piece{};
    }
    // Reset bitboards
    bitboards = {};
    for (Bitboard& bitboard : bitboards)
    {
        bitboard = 0;
    }

    // TODO: Allow FEN strings with only some information
    //  (Only placement info is needed, everything else can be set to default)
    string placement = splitString(fen, " ")[0];
    const string sideToMove = splitString(fen, " ")[1];
    const string castling = splitString(fen, " ")[2];
    const string enPassantTargetSquare = splitString(fen, " ")[3];
    const string halfMoveClock = splitString(fen, " ")[4];
    string fullMoveNumber = splitString(fen, " ")[5];

    placement = std::regex_replace(placement, std::regex("/"), "");

    int i = 0;

    for (char c : placement)
    {
        if (isdigit(c))
        {
            int n = atoi(string{c}.c_str());
            i += n - 1;
        }
        else
        {
            board[i] = Piece{c};
            addPiece(board[i], i);
        }
        i++;
    }

    whiteCanShortCastle = castling.contains("K");
    whiteCanLongCastle = castling.contains("Q");
    blackCanShortCastle = castling.contains("k");
    blackCanLongCastle = castling.contains("q");

    if (enPassantTargetSquare == "-")
    {
        this->enPassantTargetSquare = -1;
    }
    else
    {
        this->enPassantTargetSquare = square::fromString(enPassantTargetSquare);
    }

    this->sideToMove = sideToMove == "w" ? WHITE : BLACK;

    this->halfMoveClock = std::stoi(halfMoveClock);

    updateAttackingSquares();
    hashHistory.push(hash());
}

string Board::getFen() const
{
    string fen;
    int skippedSquaresCount = 0;

    for (int i = 0; i < 64; i++)
    {
        const bool goingToNextRank = i % 8 == 0 && i != 0;

        if (goingToNextRank)
        {
            if (skippedSquaresCount > 0)
            {
                fen.append(std::to_string(skippedSquaresCount));
                skippedSquaresCount = 0;
            }
            fen.append("/");
        }

        if (board[i].isNone())
        {
            skippedSquaresCount++;
        }
        else
        {
            if (skippedSquaresCount > 0)
            {
                fen.append(std::to_string(skippedSquaresCount));
                skippedSquaresCount = 0;
            }

            fen.append(board[i].toString());
        }
    }
    if (skippedSquaresCount != 0)
    {
        fen.append(std::to_string(skippedSquaresCount));
    }

    fen.append(sideToMove == WHITE ? " w " : " b ");
    fen.append("KQkq "); // TODO: castling rights
    fen.append(enPassantTargetSquare == -1 ? "-" : square::toString(enPassantTargetSquare));
    fen.append(" ").append(std::to_string(halfMoveClock));
    fen.append(" ").append(std::to_string(moveHistory.size()));

    return fen;
}

std::string Board::getPgn()
{
    /*
     This works by first creating a new board from the starting position and then playing each move in the
     reversed move history. This allows us to simply look at the piece that was on the board at that time
     to determine its type, rather than storing that information in the move. This method is rarely called, so
     performance isn't a concern.
    */

    std::string s;
    std::vector<Move> reverseMoveHistory;
    std::ranges::reverse_copy(moveHistory, reverseMoveHistory.begin());
    Board board2;
    int moveCount = 1;

    for (Move move : reverseMoveHistory)
    {

    }

    return s;
}


void Board::makeMove(Move move)
{
    boardHistory.push(BoardState{
        enPassantTargetSquare, whiteAttackingSquares, whitePawnAttackingSquares, blackAttackingSquares,
        blackPawnAttackingSquares, whiteCanShortCastle, whiteCanLongCastle, blackCanShortCastle, blackCanLongCastle,
        halfMoveClock
    });

    Piece movedPiece = board[move.start()];
    Piece capturedPiece;
    if (move.moveFlag() != MoveFlag::EnPassant)
    [[likely]]
    {
        capturedPiece = board[move.end()];
    }
    else
    {
        capturedPiece = board[sideToMove == WHITE ? move.end() + 8 : move.end() - 8];
    }

    move.capturedPiece = capturedPiece;
    moveHistory.push_back(move);

    if (capturedPiece.isNone() && movedPiece.kind() != PieceKind::PAWN)
    {
        halfMoveClock++;
    }
    else
    {
        halfMoveClock = 0;
    }

    const bool isEnPassant = move.moveFlag() == MoveFlag::EnPassant;

    // The right to capture en passant has been lost because another move has been made
    enPassantTargetSquare = -1;

    if (movedPiece.kind() == PieceKind::PAWN)
    {
        // Set the en passant target square if a pawn moved 2 squares forward
        if (move.end() == move.start() - 8 * 2)
        {
            enPassantTargetSquare = move.start() - 8;
        }
        else if (move.end() == move.start() + 8 * 2)
        {
            enPassantTargetSquare = move.start() + 8;
        }
    }

    // Update king position and castling
    if (movedPiece.kind() == PieceKind::KING)
    {
        // Castling
        if (move.moveFlag() == MoveFlag::ShortCastling || move.moveFlag() == MoveFlag::LongCastling)
        {
            // Move rook
            if (move.moveFlag() == MoveFlag::ShortCastling)
            {
                const Square rookPosition = movedPiece.color() == WHITE ? 63 : 7;
                Piece rook = board[rookPosition];
                board[rookPosition] = Piece{};
                board[move.end() - 1] = rook;
                movePiece(rook, Piece{}, rookPosition, move.end() - 1);
            }
            else if (move.moveFlag() == MoveFlag::LongCastling)
            {
                const Square rookPosition = movedPiece.color() == WHITE ? 56 : 0;
                Piece rook = board[rookPosition];
                board[rookPosition] = Piece{};
                board[move.end() + 1] = rook;
                movePiece(rook, Piece{}, rookPosition, move.end() + 1);
            }
        }

        // King has moved so castling is no longer possible
        if (movedPiece.color() == WHITE)
        {
            whiteCanShortCastle = false;
            whiteCanLongCastle = false;
        }
        else
        {
            blackCanShortCastle = false;
            blackCanLongCastle = false;
        }
    }

    // Update castling rights if rook has moved
    if (movedPiece.kind() == PieceKind::ROOK)
    {
        if (move.start() == 0)
        {
            blackCanLongCastle = false;
        }
        else if (move.start() == 7)
        {
            blackCanShortCastle = false;
        }
        else if (move.start() == 56)
        {
            whiteCanLongCastle = false;
        }
        else if (move.start() == 63)
        {
            whiteCanShortCastle = false;
        }
    }
    // Rook was captured
    if (capturedPiece.kind() == PieceKind::ROOK)
    {
        if (move.end() == 0)
        {
            blackCanLongCastle = false;
        }
        else if (move.end() == 7)
        {
            blackCanShortCastle = false;
        }
        else if (move.end() == 56)
        {
            whiteCanLongCastle = false;
        }
        else if (move.end() == 63)
        {
            whiteCanShortCastle = false;
        }
    }

    if (isEnPassant)
    {
        if (sideToMove == WHITE)
        {
            bitboards[pieceIndexes::BLACK_PAWN] &= ~bitboards::withSquare(move.end() + 8);
            board[move.end() + 8] = Piece{};
        }
        else
        {
            bitboards[pieceIndexes::WHITE_PAWN] &= ~bitboards::withSquare(move.end() - 8);
            board[move.end() - 8] = Piece{};
        }
    }

    board[move.start()] = Piece{};

    if (!move.isPromotion())
    [[likely]]
    {
        movePiece(movedPiece, capturedPiece, move.start(), move.end());
        board[move.end()] = movedPiece;
    }
    else
    {
        if (sideToMove == WHITE)
        {
            bitboards[pieceIndexes::WHITE_PAWN] &= ~bitboards::withSquare(move.start());
        }
        else
        {
            bitboards[pieceIndexes::BLACK_PAWN] &= ~bitboards::withSquare(move.start());
        }
        switch (move.moveFlag())
        {
        case MoveFlag::PromotionKnight:
            board[move.end()] = Piece{PieceKind::KNIGHT, sideToMove};
            break;
        case MoveFlag::PromotionBishop:
            board[move.end()] = Piece{PieceKind::BISHOP, sideToMove};
            break;
        case MoveFlag::PromotionRook:
            board[move.end()] = Piece{PieceKind::ROOK, sideToMove};
            break;
        case MoveFlag::PromotionQueen:
            board[move.end()] = Piece{PieceKind::QUEEN, sideToMove};
            break;
        default:
            break;
        }

        addPiece(move.moveFlag(), sideToMove, move.end());
        // Remove captured piece from bitboards
        if (!capturedPiece.isNone())
        {
            removePiece(capturedPiece, move.end());
        }
    }

    sideToMove = oppositeColor(sideToMove);

    updateAttackingSquares();
    const uint64_t newHash = hashAfterMove(move, movedPiece, capturedPiece, hashHistory.top());
    hashHistory.push(newHash);
}

void Board::makeMove(std::string uciMove)
{
    // TODO: This doesn't handle en passant (?)
    if (uciMove.length() != 4 && uciMove.length() != 5)
    {
        throw std::runtime_error{"Invalid move " + uciMove};
    }

    Square start = square::fromString(std::string{uciMove.at(0)} + uciMove.at(1));
    Square destination = square::fromString(std::string{uciMove.at(2)} + uciMove.at(3));

    MoveFlag moveFlag = MoveFlag::None;

    if (board[start].kind() == PieceKind::KING)
    {
        if (board[start].color() == BLACK && destination == 6 && blackCanShortCastle)
        {
            moveFlag = MoveFlag::ShortCastling;
        }
        else if (board[start].color() == BLACK && destination == 1 && blackCanLongCastle)
        {
            moveFlag = MoveFlag::LongCastling;
        }
        else if (board[start].color() == WHITE && destination == 62 && whiteCanShortCastle)
        {
            moveFlag = MoveFlag::ShortCastling;
        }
        else if (board[start].color() == WHITE && destination == 58 && whiteCanLongCastle)
        {
            moveFlag = MoveFlag::LongCastling;
        }
    }

    if (uciMove.length() == 5)
    {
        switch (uciMove.at(4))
        {
        case 'n':
            moveFlag = MoveFlag::PromotionKnight;
            break;
        case 'b':
            moveFlag = MoveFlag::PromotionBishop;
            break;
        case 'r':
            moveFlag = MoveFlag::PromotionRook;
            break;
        case 'q':
            moveFlag = MoveFlag::PromotionQueen;
            break;
        default:
            break;
        }
    }
    makeMove(Move{start, destination, moveFlag});
}

void Board::unmakeMove()
{
    hashHistory.pop();
    const Move move = moveHistory.back();
    moveHistory.pop_back();
    const BoardState boardState = boardHistory.top();
    boardHistory.pop();

    enPassantTargetSquare = boardState.enPassantTargetSquare;
    whiteCanShortCastle = boardState.whiteCanShortCastle;
    whiteCanLongCastle = boardState.whiteCanLongCastle;
    blackCanShortCastle = boardState.blackCanShortCastle;
    blackCanLongCastle = boardState.blackCanLongCastle;
    halfMoveClock = boardState.halfMoveClock;
    whitePawnAttackingSquares = boardState.whitePawnAttackingSquares;
    blackPawnAttackingSquares = boardState.blackPawnAttackingSquares;

    const Piece capturedPiece = move.capturedPiece;

    const bool isCapture = !capturedPiece.isNone();
    // If this was a promotion, the piece at the destination square (movedPiece) would be the piece that the pawn
    // promoted to, rather than the pawn itself, which is why this special case is needed.
    const Piece movedPiece = move.isPromotion()
                                 ? Piece{PieceKind::PAWN, oppositeColor(sideToMove)}
                                 : board[move.end()];

    // Undo castling
    if (move.moveFlag() == MoveFlag::ShortCastling)
    {
        if (movedPiece.color() == WHITE)
        {
            Piece rook = board[61];
            board[61] = Piece{};
            board[63] = rook;
            movePiece(rook, Piece{}, 61, 63);
        }
        else
        {
            Piece rook = board[5];
            board[5] = Piece{};
            board[7] = rook;
            movePiece(rook, Piece{}, 5, 7);
        }
    }
    else if (move.moveFlag() == MoveFlag::LongCastling)
    {
        if (movedPiece.color() == WHITE)
        {
            Piece rook = board[59];
            board[59] = Piece{};
            board[56] = rook;
            movePiece(rook, Piece{}, 59, 56);
        }
        else
        {
            Piece rook = board[3];
            board[3] = Piece{};
            board[0] = rook;
            movePiece(rook, Piece{}, 3, 0);
        }
    }

    board[move.end()] = {};
    board[move.start()] = movedPiece;

    movePiece(movedPiece, Piece{}, move.end(), move.start());

    if (move.isPromotion())
    {
        removePiece(move.moveFlag(), movedPiece.color(), move.end());
    }

    if (isCapture)
    {
        // In the case of en passant, the position of the captured piece will not be the destination of the move,
        // so board[move.destination()] can't be used.
        if (move.moveFlag() == MoveFlag::EnPassant)
        {
            if (sideToMove == WHITE)
            {
                // Black is the side that made the move
                // If the move is an en passant capture, the captured piece must be a pawn
                board[move.end() - 8] = Piece{PieceKind::PAWN, WHITE};
                bitboards[pieceIndexes::WHITE_PAWN] |= bitboards::withSquare(move.end() - 8);
            }
            else
            {
                // White is the side that made the move
                // If the move is an en passant capture, the captured piece must be a pawn
                board[move.end() + 8] = Piece{PieceKind::PAWN, BLACK};
                bitboards[pieceIndexes::BLACK_PAWN] |= bitboards::withSquare(move.end() + 8);
            }
        }
        else
        {
            board[move.end()] = capturedPiece;
            addPiece(capturedPiece, move.end());
        }
    }

    whiteAttackingSquares = boardState.whiteAttackingSquares;
    blackAttackingSquares = boardState.blackAttackingSquares;

    sideToMove = oppositeColor(sideToMove);
}

MoveList Board::getLegalMoves()
{
    return movegen::generateLegalMoves(*this);
}

MoveList Board::getLegalCaptures()
{
    MoveList captures{};
    for (Move move : movegen::generateLegalMoves(*this))
    {
        if (!board[move.end()].isNone() || move.moveFlag() == MoveFlag::EnPassant)
        {
            captures.push_back(move);
        }
    }
    return captures;
}

bool Board::isSideInCheckAfterMove(Move move, PieceColor side)
{
    bool isCheck = false;

    makeMove(move);
    isCheck = isSideInCheck(side);
    unmakeMove();

    return isCheck;
}

bool Board::isPseudoLegalMoveLegal(Move move)
{
    return !isSideInCheckAfterMove(move, board[move.start()].color());
}

string Board::toString() const
{
    string boardString;
    int j = 0;
    for (int i = 0; i < 64; i++)
    {
        // New line
        if (j == 8)
        {
            boardString += "  " + std::to_string(9 - i / 8) + "\n";
            j = 0;
        }
        if (board[i].isNone())
        {
            boardString += ".";
        }
        else
        {
            boardString.append(board[i].toString());
        }
        boardString.append(" ");
        j++;
    }
    boardString += "  1\n\n";
    boardString += "A B C D E F G H\n";

    return boardString;
}

std::string Board::uciMoveHistory() const
{
    std::string s;

    for (Move move : moveHistory)
    {
        s += static_cast<string>(move) + " ";
    }

    // Remove trailing space
    s = s.erase(s.size() - 1, 1);

    return s;
}

Bitboard Board::getSlidingPieces(PieceColor side) const
{
    return side == WHITE
               ? bitboards[pieceIndexes::WHITE_BISHOP] | bitboards[pieceIndexes::WHITE_ROOK] | bitboards[pieceIndexes::WHITE_QUEEN]
               : bitboards[pieceIndexes::BLACK_BISHOP] | bitboards[pieceIndexes::BLACK_ROOK] | bitboards[pieceIndexes::BLACK_QUEEN];
}

bool Board::isStalemate()
{
    return !isCheck() && getLegalMoves().empty();
}

bool Board::isInsufficientMaterial() const
{
    // TODO: Improve insufficient material detection
    using namespace pieceIndexes;

    const int whitePawnCount = std::popcount(bitboards[WHITE_PAWN]);
    const int whiteKnightCount = std::popcount(bitboards[WHITE_KNIGHT]);
    const int whiteBishopCount = std::popcount(bitboards[WHITE_BISHOP]);
    const int whiteRookCount = std::popcount(bitboards[WHITE_ROOK]);
    const int whiteQueenCount = std::popcount(bitboards[WHITE_QUEEN]);

    const int blackPawnCount = std::popcount(bitboards[BLACK_PAWN]);
    const int blackKnightCount = std::popcount(bitboards[BLACK_KNIGHT]);
    const int blackBishopCount = std::popcount(bitboards[BLACK_BISHOP]);
    const int blackRookCount = std::popcount(bitboards[BLACK_ROOK]);
    const int blackQueenCount = std::popcount(bitboards[BLACK_QUEEN]);

    if (whiteQueenCount > 0 || blackQueenCount > 0 || whiteRookCount > 0 || blackRookCount > 0 || whitePawnCount > 0 ||
        blackPawnCount > 0)
    {
        return false;
    }

    if (whiteKnightCount > 2 || blackKnightCount > 2)
    {
        return false;
    }

    if (whiteBishopCount > 2 || blackBishopCount > 2)
    {
        return false;
    }

    return true;
}

bool Board::isThreefoldRepetition()
{
    // TODO
    return false;
}

bool Board::isDraw()
{
    return halfMoveClock >= 50 || isStalemate() || isInsufficientMaterial() || isThreefoldRepetition();
}

void Board::updateAttackingSquares()
{
    using namespace pieceIndexes;
    whiteAttackingSquares = 0;
    blackAttackingSquares = 0;
    whitePawnAttackingSquares = movegen::getPawnAttackingSquares(bitboards[WHITE_PAWN], WHITE);
    blackPawnAttackingSquares = movegen::getPawnAttackingSquares(bitboards[BLACK_PAWN], BLACK);

    whiteAttackingSquares |= whitePawnAttackingSquares;
    whiteAttackingSquares |= movegen::getPieceAttackingSquares<KNIGHT>(getPieces(), bitboards[WHITE_KNIGHT]);
    whiteAttackingSquares |= movegen::getPieceAttackingSquares<BISHOP>(getPieces(), bitboards[WHITE_BISHOP]);
    whiteAttackingSquares |= movegen::getPieceAttackingSquares<ROOK>(getPieces(), bitboards[WHITE_ROOK]);
    whiteAttackingSquares |= movegen::getPieceAttackingSquares<QUEEN>(getPieces(), bitboards[WHITE_QUEEN]);
    whiteAttackingSquares |= movegen::getPieceAttackingSquares<KING>(getPieces(), bitboards[WHITE_KING]);

    blackAttackingSquares |= blackPawnAttackingSquares;
    blackAttackingSquares |= movegen::getPieceAttackingSquares<KNIGHT>(getPieces(), bitboards[BLACK_KNIGHT]);
    blackAttackingSquares |= movegen::getPieceAttackingSquares<BISHOP>(getPieces(), bitboards[BLACK_BISHOP]);
    blackAttackingSquares |= movegen::getPieceAttackingSquares<ROOK>(getPieces(), bitboards[BLACK_ROOK]);
    blackAttackingSquares |= movegen::getPieceAttackingSquares<QUEEN>(getPieces(), bitboards[BLACK_QUEEN]);
    blackAttackingSquares |= movegen::getPieceAttackingSquares<KING>(getPieces(), bitboards[BLACK_KING]);
}

std::array<uint64_t, 12 * 64 + 1 + 4 + 8> generateRandomValues()
{
    std::array<uint64_t, 12 * 64 + 1 + 4 + 8> values{};
    std::mt19937 rng; // NOLINT(*-msc51-cpp)
    std::uniform_int_distribution<uint64_t> uniformIntDistribution;

    for (uint64_t& value : values)
    {
        value = uniformIntDistribution(rng);
    }

    return values;
}

const std::array<uint64_t, 12 * 64 + 1 + 4 + 8> randomValues = generateRandomValues();

uint64_t randomValueForPiece(Piece piece, Square position)
{
    uint8_t pieceIndex = static_cast<uint8_t>(piece.kind());

    if (piece.color() == BLACK)
    {
        pieceIndex += 6;
    }

    return randomValues[(pieceIndex * 64) + position];
}

uint64_t Board::hash() const
{
    uint64_t result = 0;

    for (Square i = 0; i < 64; i++)
    {
        if (!isSquareEmpty(i))
        {
            result ^= randomValueForPiece(board[i], i);
        }
    }

    if (sideToMove == BLACK)
    {
        result ^= randomValues[(11 * 64) + 63 + 1];
    }

    if (whiteCanShortCastle)
    {
        result ^= randomValues[(11 * 64) + 63 + 2];
    }
    if (whiteCanLongCastle)
    {
        result ^= randomValues[(11 * 64) + 63 + 3];
    }
    if (blackCanShortCastle)
    {
        result ^= randomValues[(11 * 64) + 63 + 4];
    }
    if (blackCanLongCastle)
    {
        result ^= randomValues[(11 * 64) + 63 + 5];
    }

    if (enPassantTargetSquare != -1)
    {
        result ^= randomValues[(11 * 64) + 63 + 5 + square::file(enPassantTargetSquare)];
    }

    return result;
}

uint64_t Board::hashAfterMove(Move move, Piece movingPiece, Piece capturedPiece, uint64_t currentHash) const
{
    // return hash();
    // TODO


    // Remove piece from starting square
    currentHash ^= randomValueForPiece(movingPiece, move.start());
    // Add piece to new square
    currentHash ^= randomValueForPiece(movingPiece, move.end());
    // Remove captured piece
    if (!capturedPiece.isNone())
    {
        // TODO: En passant
        currentHash ^= randomValueForPiece(capturedPiece, move.end());
    }

    // Side to move has changed
    currentHash ^= randomValues[(11 * 64) + 63 + 1];

    if (move.moveFlag() == MoveFlag::ShortCastling && movingPiece.color() == WHITE)
    {
        currentHash ^= randomValues[(11 * 64) + 63 + 2];
    }
    if (move.moveFlag() == MoveFlag::LongCastling && movingPiece.color() == WHITE)
    {
        currentHash ^= randomValues[(11 * 64) + 63 + 3];
    }
    if (move.moveFlag() == MoveFlag::ShortCastling && movingPiece.color() == BLACK)
    {
        currentHash ^= randomValues[(11 * 64) + 63 + 4];
    }
    if (move.moveFlag() == MoveFlag::LongCastling && movingPiece.color() == BLACK)
    {
        currentHash ^= randomValues[(11 * 64) + 63 + 5];
    }

    if (enPassantTargetSquare != -1)
    {
        currentHash ^= randomValues[(11 * 64) + 63 + 5 + square::file(enPassantTargetSquare)];
    }

    return currentHash;
}

void Board::movePiece(Piece piece, Piece capturedPiece, Square start, Square end)
{
    // Remove captured piece
    if (!capturedPiece.isNone())
    {
        bitboards[capturedPiece.index()] &= ~bitboards::withSquare(end);
    }
    // Move piece
    bitboards[piece.index()] &= ~bitboards::withSquare(start);
    bitboards[piece.index()] |= bitboards::withSquare(end);
}

void Board::addPiece(Piece piece, Square position)
{
    bitboards[piece.index()] |= bitboards::withSquare(position);
}

void Board::addPiece(MoveFlag promotedPiece, PieceColor side, Square position)
{
    bitboards[Piece{promotedPiece, side}.index()] |= bitboards::withSquare(position);
}

void Board::removePiece(Piece piece, Square position)
{
    bitboards[piece.index()] &= ~bitboards::withSquare(position);
}

void Board::removePiece(MoveFlag promotedPiece, PieceColor side, Square position)
{
    bitboards[Piece{promotedPiece, side}.index()] &= ~bitboards::withSquare(position);
}
