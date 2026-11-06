#include <cstdio>
#include <vector>
#include "board.hpp"
#include "movegen.hpp"

int main()
{
    Board b;
    b.print();

    const char *names[] = {"Red", "Blue", "Yellow", "Green"};
    for (int p = 0; p < NUM_PLAYERS; p++)
    {
        std::vector<Move> moves;
        generateLegal(b, (Player)p, moves);
        printf("%s: %zu legal moves\n", names[p], moves.size());
    }

    std::vector<Move> red;
    generateLegal(b, RED, red);
    for (const Move &m : red)
        printf("%c%d-%c%d ", 'a' + m.fx, m.fy + 1, 'a' + m.tx, m.ty + 1);
    printf("\n");
    return 0;
}