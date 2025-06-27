#pragma once
#include "position.cpp"
#include <bits/stdc++.h>

namespace MoveGen
{
    void generate_moves(const Position &pos, std::vector<Move> &move_list);
    void generate_pawn_moves(const Position &pos, uint16_t from_sq, std::vector<Move> &move_list);
}