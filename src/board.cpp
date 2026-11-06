#include "board.hpp"
#include <cstdio>

bool Board::isValid(int x, int y)
{
    if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
        return false;
    bool xEdge = x < CORNER || x >= BOARD_SIZE - CORNER;
    bool yEdge = y < CORNER || y >= BOARD_SIZE - CORNER;
    return !(xEdge && yEdge); // both on an edge band = corner dead zone
}

void Board::setup()
{
    for (int i = 0; i < NUM_PLAYERS; i++)
        alive[i] = true;
    turn = RED;

    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            sq[y][x] = Piece{};

    // Two back rank orders: the king/queen swap so opposing armies are mirrored
    static const PieceType A[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    static const PieceType B[8] = {ROOK, KNIGHT, BISHOP, KING, QUEEN, BISHOP, KNIGHT, ROOK};

    for (int i = 0; i < 8; i++)
    {
        int c = CORNER + i; // 3..10
        put(c, 0, {A[i], RED});
        put(c, 1, {PAWN, RED}); // bottom
        put(c, 13, {B[i], YELLOW});
        put(c, 12, {PAWN, YELLOW}); // top
        put(0, c, {B[i], BLUE});
        put(1, c, {PAWN, BLUE}); // left
        put(13, c, {A[i], GREEN});
        put(12, c, {PAWN, GREEN}); // right
    }
}

void Board::print() const
{
    const char *pieces = ".PNBRQK"; // indexed by PieceType
    const char *owners = "rbyg";    // indexed by Player
    for (int y = BOARD_SIZE - 1; y >= 0; y--)
    {
        printf("%2d ", y + 1);
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            if (!isValid(x, y))
            {
                printf("   ");
                continue;
            }
            Piece p = sq[y][x];
            if (p.empty())
                printf(" . ");
            else
                printf("%c%c ", owners[p.owner], pieces[p.type]);
        }
        printf("\n");
    }
    printf("   ");
    for (int x = 0; x < BOARD_SIZE; x++)
        printf(" %c ", 'a' + x);
    printf("\n");
}

void Board::makeMove(const Move &m)
{
    Piece p = sq[m.fy][m.fx];
    if (m.promo)
        p.type = QUEEN;
    sq[m.ty][m.tx] = p;
    sq[m.fy][m.fx] = Piece{};
}