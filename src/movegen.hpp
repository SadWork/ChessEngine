#pragma once
#include "position.cpp"
#include <bits/stdc++.h>

namespace MoveGen
{
    constexpr size_t MOVE_LIST_SIZE = static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ) * static_cast<size_t>(Map::CNT_SQUARES);
    struct AttacksArray{
        std::array<Move, MOVE_LIST_SIZE> sq;
        std::array<size_t, static_cast<size_t>(Map::CNT_SQUARES)> size;
    };

    struct MoveInfo{
        Move move;
        std::shared_ptr<AttacksArray> attacks_list;
    };

    using PawnQuiteMoves = std::array<Move, static_cast<size_t>(Map::CNT_SQUARES)>;
    
    void generate_moves(Position &pos, AttacksArray &attacks_list, std::vector<MoveInfo> &move_list);

    void generate_attacks(const Position &pos, Color side_to_move, AttacksArray &attacks_list);
    void generate_pawn_moves(const Position &pos, Color side_to_move, uint16_t from_sq, PawnQuiteMoves &move_list);
    void generate_pawn_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, AttacksArray &attacks_list);
    void generate_pawn_promotions(Move move, Color side_to_move, std::vector<MoveInfo> &move_list, std::shared_ptr<AttacksArray> attacksArray);
    void generate_sliding_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, const int *directions, int num_directions, AttacksArray &attacks_list);
    void generate_leaping_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, const int *directions, int num_directions, AttacksArray &attacks_list);
    bool is_legal(const Move move, Position &pos, const AttacksArray &attacks_list, AttacksArray &next_attacks);
}

template <>
struct std::formatter<MoveGen::AttacksArray>
{
    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}')
        {
            while (it != end && *it != '}')
            {
                ++it;
            }
        }
        return it;
    }

    auto format(const MoveGen::AttacksArray &attacks, std::format_context &ctx) const
    {
        auto out = ctx.out();

        // 1. Вывод карты атак в виде доски
        out = std::format_to(out, "Attack Map (количество атакующих на клетку):\n");
        out = std::format_to(out, "  +---+---+---+---+---+---+---+---+\n");
        for (int rank = static_cast<int>(Map::HEIGHT) - 1; rank >= 0; --rank)
        {
            out = std::format_to(out, "{} |", rank + 1);
            for (int file = 0; file < static_cast<int>(Map::WIDTH); ++file)
            {
                int index = rank * static_cast<int>(Map::WIDTH) + file;
                size_t num_attacks = attacks.size[index];
                out = std::format_to(out, " {} |", num_attacks);
            }
            out = std::format_to(out, "\n  +---+---+---+---+---+---+---+---+\n");
        }
        out = std::format_to(out, "    a   b   c   d   e   f   g   h\n\n");

        // 2. Вывод детального списка атак
        out = std::format_to(out, "Детальный список атак:\n");
        for (int i = 0; i < static_cast<int>(Map::CNT_SQUARES); ++i)
        {
            if (attacks.size[i] > 0)
            {
                out = std::format_to(out, "Клетка {}: {} атак(и) с полей [", FEN::index_to_square(i), attacks.size[i]);
                for (size_t j = 0; j < attacks.size[i]; ++j)
                {
                    const Move &m = attacks.sq[i * static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ) + j];
                    out = std::format_to(out, "{}", FEN::index_to_square(m.source()));
                    if (j < attacks.size[i] - 1) {
                        out = std::format_to(out, ", ");
                    }
                }
                out = std::format_to(out, "]\n");
            }
        }
        return out;
    }
};