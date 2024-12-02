#include "Board.hpp"

#include <random>
#include <regex>

#include "movegen.hpp"
#include "utils.hpp"

using std::string;

void Board::loadFen(string fen)
{
    hashHistory = {};
    for (int i = 0; i < 64; i++)
    {
        board[i] = Piece{};
    }
    // Reset bitboards
    whitePawns = 0;
    whiteKnights = 0;
    whiteBishops = 0;
    whiteRooks = 0;
    whiteQueens = 0;
    whiteKing = 0;
    blackPawns = 0;
    blackKnights = 0;
    blackBishops = 0;
    blackRooks = 0;
    blackQueens = 0;
    blackKing = 0;

    // TODO: Allow FEN strings with only some information
    //  (Only placement info is needed, everything else can be set to default)
    string placement = splitString(fen, " ")[0];
    string sideToMove = splitString(fen, " ")[1];
    string castling = splitString(fen, " ")[2];
    string enPassantTargetSquare = splitString(fen, " ")[3];
    string halfMoveClock = splitString(fen, " ")[4];
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
            if (c == 'K')
            {
                whiteKingPosition = i;
            }
            else if (c == 'k')
            {
                blackKingPosition = i;
            }
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

    if (movedPiece.kind == PieceKind::KING)
    {
        if (sideToMove == WHITE)
        {
            whiteKingPosition = move.end();
        }
        else
        {
            blackKingPosition = move.end();
        }
    }

    move.capturedPiece = capturedPiece;
    moveHistory.push_back(move);

    if (capturedPiece.isNone() && movedPiece.kind != PieceKind::PAWN)
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

    if (movedPiece.kind == PieceKind::PAWN)
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
    if (movedPiece.kind == PieceKind::KING)
    {
        // Castling
        if (move.moveFlag() == MoveFlag::ShortCastling || move.moveFlag() == MoveFlag::LongCastling)
        {
            // Move rook
            if (move.moveFlag() == MoveFlag::ShortCastling)
            {
                const Square rookPosition = movedPiece.color == WHITE ? 63 : 7;
                Piece rook = board[rookPosition];
                board[rookPosition] = Piece{};
                board[move.end() - 1] = rook;
                movePiece(rook, Piece{}, rookPosition, move.end() - 1);
            }
            else if (move.moveFlag() == MoveFlag::LongCastling)
            {
                const Square rookPosition = movedPiece.color == WHITE ? 56 : 0;
                Piece rook = board[rookPosition];
                board[rookPosition] = Piece{};
                board[move.end() + 1] = rook;
                movePiece(rook, Piece{}, rookPosition, move.end() + 1);
            }
        }

        // King has moved so castling is no longer possible
        if (movedPiece.color == WHITE)
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
    if (movedPiece.kind == PieceKind::ROOK)
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
    if (capturedPiece.kind == PieceKind::ROOK)
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
            blackPawns &= ~bitboards::withSquare(move.end() + 8);
            board[move.end() + 8] = Piece{};
        }
        else
        {
            whitePawns &= ~bitboards::withSquare(move.end() - 8);
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
            whitePawns &= ~bitboards::withSquare(move.start());
        }
        else
        {
            blackPawns &= ~bitboards::withSquare(move.start());
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
        if (capturedPiece.kind != PieceKind::NONE)
        {
            removePiece(capturedPiece, move.end());
        }
    }

    sideToMove = oppositeColor(sideToMove);

    updateAttackingSquares();
    hashHistory.push(hash());
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

    if (board[start].kind == PieceKind::KING)
    {
        if (board[start].color == BLACK && destination == 6 && blackCanShortCastle)
        {
            moveFlag = MoveFlag::ShortCastling;
        }
        else if (board[start].color == BLACK && destination == 1 && blackCanLongCastle)
        {
            moveFlag = MoveFlag::LongCastling;
        }
        else if (board[start].color == WHITE && destination == 62 && whiteCanShortCastle)
        {
            moveFlag = MoveFlag::ShortCastling;
        }
        else if (board[start].color == WHITE && destination == 58 && whiteCanLongCastle)
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

    // if (board[square::fromString("h7")].isNone() && board[square::fromString("h6")].isNone() && board[square::fromString("h5")].isNone())
    // {
    //     std::cout << "c unmake start" << "\n";
    // }

    const bool isCapture = !capturedPiece.isNone();
    // If this was a promotion, the piece at the destination square (movedPiece) would be the piece that the pawn
    // promoted to, rather than the pawn itself, which is why this special case is needed.
    const Piece movedPiece = move.isPromotion()
                                 ? Piece{PieceKind::PAWN, oppositeColor(sideToMove)}
                                 : board[move.end()];

    if (movedPiece.kind == PieceKind::KING)
    {
        if (movedPiece.color == WHITE)
        {
            whiteKingPosition = move.start();
        }
        else
        {
            blackKingPosition = move.start();
        }
    }

    // Undo castling
    if (move.moveFlag() == MoveFlag::ShortCastling)
    {
        if (movedPiece.color == WHITE)
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
        if (movedPiece.color == WHITE)
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
        removePiece(move.moveFlag(), movedPiece.color, move.end());
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
                whitePawns |= bitboards::withSquare(move.end() - 8);
            }
            else
            {
                // White is the side that made the move
                // If the move is an en passant capture, the captured piece must be a pawn
                board[move.end() + 8] = Piece{PieceKind::PAWN, BLACK};
                blackPawns |= bitboards::withSquare(move.end() + 8);
            }
        }
        else
        {
            board[move.end()] = capturedPiece;
            addPiece(capturedPiece, move.end());
        }
    }

    // if (board[square::fromString("h7")].isNone() && board[
    //     square::fromString("h6")].isNone() && board[square::fromString("h5")].isNone())
    // {
    //     std::cout << "c unmake end" << "\n";
    // }

    whiteAttackingSquares = boardState.whiteAttackingSquares;
    blackAttackingSquares = boardState.blackAttackingSquares;

    sideToMove = oppositeColor(sideToMove);
}

std::vector<Move> Board::getLegalMoves()
{
    return generateLegalMoves(*this);
}

std::vector<Move> Board::getLegalCaptures()
{
    std::vector<Move> captures{};
    captures.reserve(10); // Completely arbitrary, it's probably better to over-allocate than reallocating multiple times
    for (Move move : generateLegalMoves(*this))
    {
        if (!board[move.end()].isNone() || move.moveFlag() == MoveFlag::EnPassant)
        {
            captures.push_back(move);
        }
    }
    return captures;
}

void Board::getPseudoLegalMoves(std::vector<Move>& moves) const
{
    for (Square i = 0; i < 64; i++)
    {
        const Piece piece = board[i];
        if (!piece.isNone() && piece.color == sideToMove)
        {
            generatePseudoLegalMoves(piece, i, *this, moves);
        }
    }
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
    return !isSideInCheckAfterMove(move, board[move.start()].color);
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
               ? whiteBishops | whiteRooks | whiteQueens
               : blackBishops | blackRooks | blackQueens;
}

bool Board::isStalemate()
{
    return !isCheck() && getLegalMoves().empty();
}

bool Board::isInsufficientMaterial() const
{
    // TODO: Improve insufficient material detection

    const int whitePawnCount = std::popcount(whitePawns);
    const int whiteKnightCount = std::popcount(whiteKnights);
    const int whiteBishopCount = std::popcount(whiteBishops);
    const int whiteRookCount = std::popcount(whiteRooks);
    const int whiteQueenCount = std::popcount(whiteQueens);

    const int blackPawnCount = std::popcount(blackPawns);
    const int blackKnightCount = std::popcount(blackKnights);
    const int blackBishopCount = std::popcount(blackBishops);
    const int blackRookCount = std::popcount(blackRooks);
    const int blackQueenCount = std::popcount(blackQueens);

    if (whiteQueenCount > 0 || blackQueenCount > 0 || whiteRookCount > 0 || blackRookCount > 0 || whitePawnCount > 0 || blackPawnCount > 0) {
        return false;
    }

    if (whiteKnightCount > 2 || blackKnightCount > 2) {
        return false;
    }

    if (whiteBishopCount > 2 || blackBishopCount > 2) {
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
    whiteAttackingSquares = 0;
    blackAttackingSquares = 0;
    whitePawnAttackingSquares = 0;
    blackPawnAttackingSquares = 0;

    for (Square i = 0; i < 64; i++)
    {
        const Piece piece = board[i];

        if (piece.isNone())
        {
            continue;
        }

        const Bitboard attackingSquares = generateAttackingSquares(piece, i, *this);

        if (piece.color == WHITE)
        {
            whiteAttackingSquares |= attackingSquares;
            if (piece.kind == PieceKind::PAWN)
            {
                whitePawnAttackingSquares |= attackingSquares;
            }
        }
        else
        {
            blackAttackingSquares |= attackingSquares;
            if (piece.kind == PieceKind::PAWN)
            {
                blackPawnAttackingSquares |= attackingSquares;
            }
        }
    }
}

std::array<uint64_t, 12 * 64 + 1 + 4 + 8> generateRandomValues()
{
    std::array<uint64_t, 12 * 64 + 1 + 4 + 8> values{};
    std::mt19937 rng;
    std::uniform_int_distribution<uint64_t> uniformIntDistribution;

    for (uint64_t& value : values)
    {
        value = uniformIntDistribution(rng);
    }

    return values;
}

const std::array<uint64_t, 12 * 64 + 1 + 4 + 8> randomValues = generateRandomValues();

uint64_t Board::hash() const
{
    uint64_t result = 0;

    for (Square i = 0; i < 64; i++)
    {
        if (isSquareEmpty(i))
        {
            continue;
        }

        // Subtract 1 because this should start at 0 and pieces start at 1 (0 is PieceKind::NONE)
        uint8_t pieceIndex = static_cast<uint8_t>(board[i].kind) - 1;

        if (board[i].color == BLACK)
        {
            pieceIndex += 6;
        }

        result ^= randomValues[(pieceIndex * 64) + i];
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

void Board::movePiece(Piece piece, Piece capturedPiece, Square start, Square end)
{
    // Remove captured piece
    switch (capturedPiece.kind)
    {
    case PieceKind::NONE:
        break;
    case PieceKind::PAWN:
        if (capturedPiece.color == WHITE)
        {
            whitePawns &= ~bitboards::withSquare(end);
        }
        else
        {
            blackPawns &= ~bitboards::withSquare(end);
        }
        break;
    case PieceKind::KNIGHT:
        if (capturedPiece.color == WHITE)
        {
            whiteKnights &= ~bitboards::withSquare(end);
        }
        else
        {
            blackKnights &= ~bitboards::withSquare(end);
        }
        break;
    case PieceKind::BISHOP:
        if (capturedPiece.color == WHITE)
        {
            whiteBishops &= ~bitboards::withSquare(end);
        }
        else
        {
            blackBishops &= ~bitboards::withSquare(end);
        }
        break;
    case PieceKind::ROOK:
        if (capturedPiece.color == WHITE)
        {
            whiteRooks &= ~bitboards::withSquare(end);
        }
        else
        {
            blackRooks &= ~bitboards::withSquare(end);
        }
        break;
    case PieceKind::QUEEN:
        if (capturedPiece.color == WHITE)
        {
            whiteQueens &= ~bitboards::withSquare(end);
        }
        else
        {
            blackQueens &= ~bitboards::withSquare(end);
        }
        break;
    case PieceKind::KING:
        break;
    }

    // Move piece

    switch (piece.kind)
    {
    case PieceKind::NONE:
        break;
    case PieceKind::PAWN:
        if (piece.color == WHITE)
        {
            whitePawns &= ~(bitboards::withSquare(start));
            whitePawns |= bitboards::withSquare(end);
        }
        else
        {
            blackPawns &= ~(bitboards::withSquare(start));
            blackPawns |= bitboards::withSquare(end);
        }
        break;
    case PieceKind::KNIGHT:
        if (piece.color == WHITE)
        {
            whiteKnights &= ~(bitboards::withSquare(start));
            whiteKnights |= bitboards::withSquare(end);
        }
        else
        {
            blackKnights &= ~(bitboards::withSquare(start));
            blackKnights |= bitboards::withSquare(end);
        }
        break;
    case PieceKind::BISHOP:
        if (piece.color == WHITE)
        {
            whiteBishops &= ~(bitboards::withSquare(start));
            whiteBishops |= bitboards::withSquare(end);
        }
        else
        {
            blackBishops &= ~(bitboards::withSquare(start));
            blackBishops |= bitboards::withSquare(end);
        }
        break;
    case PieceKind::ROOK:
        if (piece.color == WHITE)
        {
            whiteRooks &= ~(bitboards::withSquare(start));
            whiteRooks |= bitboards::withSquare(end);
        }
        else
        {
            blackRooks &= ~(bitboards::withSquare(start));
            blackRooks |= bitboards::withSquare(end);
        }
        break;
    case PieceKind::QUEEN:
        if (piece.color == WHITE)
        {
            whiteQueens &= ~(bitboards::withSquare(start));
            whiteQueens |= bitboards::withSquare(end);
        }
        else
        {
            blackQueens &= ~(bitboards::withSquare(start));
            blackQueens |= bitboards::withSquare(end);
        }
        break;
    case PieceKind::KING:
        if (piece.color == WHITE)
        {
            whiteKing &= ~(bitboards::withSquare(start));
            whiteKing |= bitboards::withSquare(end);
        }
        else
        {
            blackKing &= ~(bitboards::withSquare(start));
            blackKing |= bitboards::withSquare(end);
        }
        break;
    }
}

void Board::addPiece(Piece piece, Square position)
{
    if (piece.color == WHITE)
    {
        switch (piece.kind)
        {
        case PieceKind::PAWN:
            whitePawns |= bitboards::withSquare(position);
            break;
        case PieceKind::KNIGHT:
            whiteKnights |= bitboards::withSquare(position);
            break;
        case PieceKind::BISHOP:
            whiteBishops |= bitboards::withSquare(position);
            break;
        case PieceKind::ROOK:
            whiteRooks |= bitboards::withSquare(position);
            break;
        case PieceKind::QUEEN:
            whiteQueens |= bitboards::withSquare(position);
            break;
        case PieceKind::KING:
            whiteKing |= bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (piece.kind)
        {
        case PieceKind::PAWN:
            blackPawns |= bitboards::withSquare(position);
            break;
        case PieceKind::KNIGHT:
            blackKnights |= bitboards::withSquare(position);
            break;
        case PieceKind::BISHOP:
            blackBishops |= bitboards::withSquare(position);
            break;
        case PieceKind::ROOK:
            blackRooks |= bitboards::withSquare(position);
            break;
        case PieceKind::QUEEN:
            blackQueens |= bitboards::withSquare(position);
            break;
        case PieceKind::KING:
            blackKing |= bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
}


void Board::addPiece(MoveFlag promotedPiece, PieceColor side, Square position)
{
    if (side == WHITE)
    {
        switch (promotedPiece)
        {
        case MoveFlag::PromotionKnight:
            whiteKnights |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionBishop:
            whiteBishops |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionRook:
            whiteRooks |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionQueen:
            whiteQueens |= bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (promotedPiece)
        {
        case MoveFlag::PromotionKnight:
            blackKnights |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionBishop:
            blackBishops |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionRook:
            blackRooks |= bitboards::withSquare(position);
            break;
        case MoveFlag::PromotionQueen:
            blackQueens |= bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
}

void Board::removePiece(Piece piece, Square position)
{
    if (piece.color == WHITE)
    {
        switch (piece.kind)
        {
        case PieceKind::PAWN:
            whitePawns &= ~bitboards::withSquare(position);
            break;
        case PieceKind::KNIGHT:
            whiteKnights &= ~bitboards::withSquare(position);
            break;
        case PieceKind::BISHOP:
            whiteBishops &= ~bitboards::withSquare(position);
            break;
        case PieceKind::ROOK:
            whiteRooks &= ~bitboards::withSquare(position);
            break;
        case PieceKind::QUEEN:
            whiteQueens &= ~bitboards::withSquare(position);
            break;
        case PieceKind::KING:
            whiteKing &= ~bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (piece.kind)
        {
        case PieceKind::PAWN:
            blackPawns &= ~bitboards::withSquare(position);
            break;
        case PieceKind::KNIGHT:
            blackKnights &= ~bitboards::withSquare(position);
            break;
        case PieceKind::BISHOP:
            blackBishops &= ~bitboards::withSquare(position);
            break;
        case PieceKind::ROOK:
            blackRooks &= ~bitboards::withSquare(position);
            break;
        case PieceKind::QUEEN:
            blackQueens &= ~bitboards::withSquare(position);
            break;
        case PieceKind::KING:
            blackKing &= ~bitboards::withSquare(position);
            break;
        default:
            break;
        }
    }
}

void Board::removePiece(MoveFlag promotedPiece, PieceColor side, Square position)
{
    PieceKind pieceKind;
    switch (promotedPiece)
    {
    case MoveFlag::PromotionKnight:
        pieceKind = PieceKind::KNIGHT;
        break;
    case MoveFlag::PromotionBishop:
        pieceKind = PieceKind::BISHOP;
        break;
    case MoveFlag::PromotionRook:
        pieceKind = PieceKind::ROOK;
        break;
    case MoveFlag::PromotionQueen:
        pieceKind = PieceKind::QUEEN;
        break;
    default:
        return;
    }
    removePiece(Piece{pieceKind, side}, position);
}
