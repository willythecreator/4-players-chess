#include "movegen.hpp"

namespace
{

    const int KNIGHT_D[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
    // Even indices are orthogonal, odd indices are diagonal
    const int KING_D[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
    const int ORTHO[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    const int DIAG[4][2] = {{1, 1}, {-1, 1}, {-1, -1}, {1, -1}};

    // 0-based rank from the pawn's own side where it promotes (10 = the 11th rank)
    constexpr int PROMO_RANK = 10;

    Move mk(int fx, int fy, int tx, int ty, bool promo = false)
    {
        return Move{(uint8_t)fx, (uint8_t)fy, (uint8_t)tx, (uint8_t)ty, promo};
    }

    // How far a pawn of player p is from its own back edge (starting pawns are at 1)
    int pawnRank(Player p, int x, int y)
    {
        switch (p)
        {
        case RED:
            return y;
        case BLUE:
            return x;
        case YELLOW:
            return BOARD_SIZE - 1 - y;
        default:
            return BOARD_SIZE - 1 - x;
        }
    }

    // Kings can never be captured: a player with no escape is checkmated on their own turn
    bool canCapture(const Piece &t, Player me)
    {
        return !t.empty() && t.owner != me && t.type != KING;
    }

    void genStep(const Board &b, Player me, int x, int y,
                 const int (*d)[2], int n, std::vector<Move> &out)
    {
        for (int i = 0; i < n; i++)
        {
            int tx = x + d[i][0], ty = y + d[i][1];
            if (!Board::isValid(tx, ty))
                continue;
            Piece t = b.get(tx, ty);
            if (t.empty() || canCapture(t, me))
                out.push_back(mk(x, y, tx, ty));
        }
    }

    void genSlide(const Board &b, Player me, int x, int y,
                  const int (*d)[2], int n, std::vector<Move> &out)
    {
        for (int i = 0; i < n; i++)
        {
            int tx = x + d[i][0], ty = y + d[i][1];
            while (Board::isValid(tx, ty))
            {
                Piece t = b.get(tx, ty);
                if (t.empty())
                {
                    out.push_back(mk(x, y, tx, ty));
                }
                else
                {
                    if (canCapture(t, me))
                        out.push_back(mk(x, y, tx, ty));
                    break;
                }
                tx += d[i][0];
                ty += d[i][1];
            }
        }
    }

    void genPawn(const Board &b, Player me, int x, int y, std::vector<Move> &out)
    {
        int fx = FWD_X[me], fy = FWD_Y[me];
        int px = (fy != 0) ? 1 : 0; // sideways vector, perpendicular to forward
        int py = (fx != 0) ? 1 : 0;

        auto add = [&](int ax, int ay)
        {
            out.push_back(mk(x, y, ax, ay, pawnRank(me, ax, ay) >= PROMO_RANK));
        };

        int tx = x + fx, ty = y + fy;
        if (Board::isValid(tx, ty) && b.get(tx, ty).empty())
        {
            add(tx, ty);
            if (pawnRank(me, x, y) == 1)
            { // double step from the starting line
                int t2x = tx + fx, t2y = ty + fy;
                if (Board::isValid(t2x, t2y) && b.get(t2x, t2y).empty())
                    add(t2x, t2y);
            }
        }

        for (int s = -1; s <= 1; s += 2)
        {
            int cx = x + fx + s * px, cy = y + fy + s * py;
            if (!Board::isValid(cx, cy))
                continue;
            if (canCapture(b.get(cx, cy), me))
                add(cx, cy);
        }
    }

} // namespace

// Is square (x,y) attacked by any living player other than `defender`?
// Pieces of eliminated players never attack, but they still block.
bool isAttacked(const Board &b, int x, int y, Player defender)
{
    auto hostile = [&](int ax, int ay, PieceType type)
    {
        if (!Board::isValid(ax, ay))
            return false;
        Piece t = b.get(ax, ay);
        return t.type == type && t.owner != defender && b.alive[t.owner];
    };

    for (int i = 0; i < 8; i++)
    {
        if (hostile(x + KNIGHT_D[i][0], y + KNIGHT_D[i][1], KNIGHT))
            return true;
        if (hostile(x + KING_D[i][0], y + KING_D[i][1], KING))
            return true;
    }

    // Pawns: look backwards along each attacker's own forward direction
    for (int q = 0; q < NUM_PLAYERS; q++)
    {
        if (q == defender || !b.alive[q])
            continue;
        int px = (FWD_Y[q] != 0) ? 1 : 0;
        int py = (FWD_X[q] != 0) ? 1 : 0;
        for (int s = -1; s <= 1; s += 2)
        {
            int ax = x - FWD_X[q] + s * px;
            int ay = y - FWD_Y[q] + s * py;
            if (!Board::isValid(ax, ay))
                continue;
            Piece t = b.get(ax, ay);
            if (t.type == PAWN && t.owner == q)
                return true;
        }
    }

    // Sliders: walk outward until the first piece
    for (int d = 0; d < 8; d++)
    {
        bool diagonal = (d & 1) != 0;
        int ax = x + KING_D[d][0], ay = y + KING_D[d][1];
        while (Board::isValid(ax, ay))
        {
            Piece t = b.get(ax, ay);
            if (!t.empty())
            {
                if (t.owner != defender && b.alive[t.owner])
                {
                    if (t.type == QUEEN)
                        return true;
                    if (diagonal ? t.type == BISHOP : t.type == ROOK)
                        return true;
                }
                break;
            }
            ax += KING_D[d][0];
            ay += KING_D[d][1];
        }
    }
    return false;
}

bool inCheck(const Board &b, Player p)
{
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            Piece t = b.get(x, y);
            if (t.type == KING && t.owner == p)
                return isAttacked(b, x, y, p);
        }
    return false;
}

void generatePseudoLegal(const Board &b, Player me, std::vector<Move> &out)
{
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            if (!Board::isValid(x, y))
                continue;
            Piece pc = b.get(x, y);
            if (pc.empty() || pc.owner != me)
                continue;
            switch (pc.type)
            {
            case PAWN:
                genPawn(b, me, x, y, out);
                break;
            case KNIGHT:
                genStep(b, me, x, y, KNIGHT_D, 8, out);
                break;
            case KING:
                genStep(b, me, x, y, KING_D, 8, out);
                break;
            case BISHOP:
                genSlide(b, me, x, y, DIAG, 4, out);
                break;
            case ROOK:
                genSlide(b, me, x, y, ORTHO, 4, out);
                break;
            case QUEEN:
                genSlide(b, me, x, y, DIAG, 4, out);
                genSlide(b, me, x, y, ORTHO, 4, out);
                break;
            default:
                break;
            }
        }
}

void generateLegal(const Board &b, Player p, std::vector<Move> &out)
{
    std::vector<Move> pseudo;
    generatePseudoLegal(b, p, pseudo);
    for (const Move &m : pseudo)
    {
        Board next = b; // copy-make: simple now, we'll switch to make/unmake for the bot
        next.makeMove(m);
        if (!inCheck(next, p))
            out.push_back(m);
    }
}