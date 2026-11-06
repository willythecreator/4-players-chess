#pragma once
#include "types.hpp"

class Board
{
public:
    Board() { setup(); }

    static bool isValid(int x, int y);

    Piece get(int x, int y) const { return sq[y][x]; }
    void put(int x, int y, Piece p) { sq[y][x] = p; }

    void setup();
    void print() const;
    void makeMove(const Move &m); // no legality check, just moves the piece

    bool alive[NUM_PLAYERS];
    Player turn;

private:
    Piece sq[BOARD_SIZE][BOARD_SIZE]; // sq[y][x], y = 0 is Red's side
};