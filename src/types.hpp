#pragma once
#include <cstdint>

constexpr int BOARD_SIZE = 14;
constexpr int CORNER = 3; // each corner is a 3x3 dead zone
constexpr int NUM_PLAYERS = 4;

enum Player : uint8_t
{
    RED,
    BLUE,
    YELLOW,
    GREEN
};

enum PieceType : uint8_t
{
    NONE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

struct Piece
{
    PieceType type = NONE;
    Player owner = RED;
    bool empty() const { return type == NONE; }
};

struct Move
{
    uint8_t fx, fy, tx, ty;
    bool promo;
};

// Forward direction of each player's pawns, indexed by Player
constexpr int FWD_X[NUM_PLAYERS] = {0, 0, 0, -1};
constexpr int FWD_Y[NUM_PLAYERS] = {1, 0, -1, 0};
