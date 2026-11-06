#pragma once
#include <vector>
#include "board.hpp"

bool isAttacked(const Board &b, int x, int y, Player defender);
bool inCheck(const Board &b, Player p);
void generatePseudoLegal(const Board &b, Player p, std::vector<Move> &out);
void generateLegal(const Board &b, Player p, std::vector<Move> &out);