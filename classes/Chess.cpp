#include "Chess.h"
#include <limits>
#include <cmath>

Chess::Chess()
{
    _grid = new Grid(8, 8);
}

Chess::~Chess()
{
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    ChessSquare* square = _grid->getSquare(x, y);
    Bit *bit = square->bit();
    char notation = '0';
    if (bit != nullptr) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }

    //std::cout << "Square: " << notation << std::endl;

    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);
    bit->setGameTag(piece);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board

    // FEN is a space delimited string with 6 fields

    std::stringstream s(fen);
    std::string row;

    char rowDelimiter = '/';

    int rank = _grid->getHeight() - 1; // Chess Rank (row)

    while (getline(s, row, rowDelimiter))
    {
        int file = 0; // Chess Files (column)

        for (char c : row)
        {
            ChessSquare* square = _grid->getSquare(file, rank);
            ChessPiece piece;

            // I needed help with this one. I found the isdigit function and
            // then just needed to increase file by the number of squares,
            // and continue the loop.

            if (std::isdigit(c)) // Skips c squares if empty
            {
                file += c;
                continue;
            }

            // Realized switch statements were a cleaner solution during
            // my C++ syntax research while debugging the out-of-range error.

            switch (std::tolower(c)) // Equalizes all piece codes to assign values
            {
                case 'r': piece = Rook; break;
                case 'n': piece = Knight; break;
                case 'b': piece = Bishop; break;
                case 'k': piece = King; break;
                case 'q': piece = Queen; break;
                case 'p': piece = Pawn; break;
                default: continue;
            }

            // I had found I was getting an out-of-range error when I tried
            // passing the playerNumber, and realized I was passing -1, per
            // AIPlayer. I simply hard-coded 1 and 0 for the below function. 

            int pieceOwner = std::isupper(c) ? 0 : 1; // Detects if piece is white / black

            Bit *bit = PieceForPlayer(pieceOwner, piece);

            if (bit != nullptr)
            {
                bit->setPosition(square->getPosition());
                square->setBit(bit);
            }

            std::cout << "Player: " << bit->getOwner()->playerNumber() << std::endl;

            std::cout << "Square: " << pieceNotation(file, rank) << " - (" << file << ", " << rank << ")" << std::endl;

            file++;
        }

        rank--;
    }

    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceOwner = bit.getOwner()->playerNumber() * 128;

    //std::cout << "Piece Owner: " << (pieceOwner == currentPlayer) << std::endl;

    if (pieceOwner == currentPlayer) return true;
    return false;
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return true;
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) 
        {
            int newy = 7 - y;
            s += pieceNotation( x, newy );
        }
    );

    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}
