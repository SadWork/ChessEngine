#pragma once
#include "position.cpp"
#include <bits/stdc++.h>

namespace MoveGen
{
    void generate_moves(const Position &pos, std::vector<Move> &move_list);
    void generate_pawn_moves(const Position &pos, uint16_t from_sq, std::vector<Move> &move_list);
    void generate_pawn_attacks(const Position &pos, uint16_t from_sq, std::vector<Move> &move_list);
    bool generate_pawn_promotions(uint16_t from_sq, uint16_t dest_sq, uint16_t push_twice_dest,std::vector<Move> &move_list);
    void generate_sliding_attacks(const Position &pos, uint16_t from_sq, const int *directions, int num_directions, std::vector<Move> &move_list);
    void generate_leaping_attacks(const Position &pos, uint16_t from_sq, const int *directions, int num_directions, std::vector<Move> &move_list);
}