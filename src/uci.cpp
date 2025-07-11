#pragma once
#include <bits/stdc++.h>
#include "types.h"
#include "position.hpp"
#include "movegen.hpp"

namespace UCI
{
#include "uci_commands.hpp"
    Position g_position;
    bool quit_flag = false;
    bool stop_search = true;

    Move parse_uci_move(std::string_view sv)
    {
        const unsigned char from_sq = FEN::square_to_index(sv.substr(0, 2));
        const unsigned char to_sq = FEN::square_to_index(sv.substr(2, 2));

        const unsigned char moved_piece_idx = g_position.board[from_sq];
        const unsigned char moved_piece = g_position.pieces_list[moved_piece_idx].type;
        PieceType moved_piece_type = FEN::get_piece_type(moved_piece);

        const unsigned char from_file = from_sq & 0x7;
        const unsigned char to_file = to_sq & 0x7;
        const unsigned char file_dist = from_file > to_file ? from_file - to_file : to_file - from_file;

        if (sv.length() == 5)
        {
            PieceType promotion_pt = PieceType::EMPTY;
            switch (sv[4])
            { // Символ фигуры превращения (q, r, b, n)
            case 'q':
                promotion_pt = PieceType::QUEEN;
                break;
            case 'r':
                promotion_pt = PieceType::ROOK;
                break;
            case 'b':
                promotion_pt = PieceType::BISHOP;
                break;
            case 'n':
                promotion_pt = PieceType::KNIGHT;
                break;
            default:
                break;
            }
            return Move(from_sq, to_sq, promotion_pt);
        }
        else if (moved_piece_type == PieceType::KING && file_dist > 1)
        {
            return Move(from_sq, to_sq, MoveType::CASTLING);
        }
        else if (moved_piece_type == PieceType::PAWN && to_sq == g_position.enpassant_target_square)
        {
            return Move(from_sq, to_sq, MoveType::EN_PASSANT);
        }
        return Move(from_sq, to_sq, MoveType::NORMAL);
    }

    void handle_uci()
    {
        static constexpr char EngineName[] = "DumpFish";
        static constexpr char AuthorName[] = "SadWork";
        static constexpr char Version[] = "1.0";

        std::println("id name {} {}", EngineName, Version);
        std::println("id author {}", AuthorName);
        std::println("uciok");
    };

    void handle_isready() { std::println("readyok"); };

    void handle_position()
    {
        std::string token;

        std::string line;
        std::getline(std::cin, line);
        std::stringstream ss(line);

        if (!(ss >> token))
        {
            std::println("info string Error: Missing format ('startpos' or 'fen') after 'position'");
        }

        if (token == "startpos")
        {
            g_position.set_from_fen(FEN::Default);
        }
        else if (token == "fen")
        {
            std::string fen_string;
            for (int i = 0; i < 6; ++i)
            {
                std::string fen_part;
                if (!(ss >> fen_part))
                {
                    std::println("info string Error: Incomplete FEN string provided.");
                    return;
                }
                fen_string += fen_part + " ";
            }
            g_position.set_from_fen(fen_string);
        }
        else
        {
            std::println("info string Error: Unknown format '{}' after 'position'. Expected 'startpos' or 'fen'.", token);
            return;
        }
        if (ss >> token)
        {
            if (token != "moves")
            {
                std::println("info string Error: Unknown format '{}' after 'positon <fen>'. Expected 'moves'", token);
                return;
            }
            while (ss >> token)
            {
                Move m = parse_uci_move(token);
                g_position.do_move(m);
            }
        }
    };

    void handle_go()
    {
        stop_search = false;
        std::println("{}", __PRETTY_FUNCTION__);
    };

    void handle_stop()
    {
        stop_search = true;
        std::println("{}", __PRETTY_FUNCTION__);
    };

    void handle_quit_wrapper()
    {
        handle_stop();
        quit_flag = true;
    };

    void handle_ucinewgame() { std::println("{}", __PRETTY_FUNCTION__); };

    void uci_loop()
    {
        Perfect_Hash command_finder;
        std::string token;
        while (!(quit_flag) && std::cin >> token)
        {
            const UciCommandAction *action = command_finder.in_word_set(token.c_str(), token.length());
            if (action != nullptr)
            {
                action->handler();
            }
            else
            {
                std::println("info string Unknown command: {}", token);
            }
        }
    }

#ifdef DEBUG
void handle_print_pos() { std::println("{}", g_position); }

void undo_last_move()
{
    g_position.undo_move();
}

uint64_t perft(Position& pos, int depth, std::shared_ptr<MoveGen::AttacksArray> attacks_list) {
    // static std::string path = "";
    static std::vector<MoveGen::MoveInfo> move_list;
    
    if (depth == 0) {
        // std::println("{{{}}}", path);
        return 1;
    }
    
    // std::print("{}", *attacks_list);
    size_t i_from = move_list.size(); 
    MoveGen::generate_moves(pos, *attacks_list, move_list);
    size_t i_to = move_list.size();

    uint64_t nodes = 0;
    for(size_t i = i_from; i < i_to; ++i) {
        Move m = move_list[i].move;
        // size_t prev_path_size = path.size();
        // path += std::string(FEN::index_to_square(m.source()))
        //                    + "->"
        //                    + std::string(FEN::index_to_square(m.dest()));
        // if(m.type() == MoveType::PROMOTION) {
        //     path+= FEN::piece_to_fen_char(static_cast<uint16_t>(m.promotion_piece()));
        // }
        pos.do_move(m);
        nodes += perft(pos, depth - 1, move_list[i].attacks_list);
        // path.resize(prev_path_size);
        pos.undo_move();
    }
    move_list.resize(i_from);

    return nodes;
}

void handle_perft() {
        // ... парсинг глубины
        int depth;
        std::cin>>depth;

        auto attacks_list = std::make_shared<MoveGen::AttacksArray>();
        Color cur_color = static_cast<Color>(Position::get_side_to_move(g_position));
        MoveGen::generate_attacks(g_position, cur_color, *attacks_list);
        
        std::println("{}", *attacks_list);
        uint64_t nodes = perft(g_position, depth, attacks_list);
        std::println("Nodes searched: {}", nodes);
    }
#endif
}

int main(const int argc, const char *argv[])
{
    UCI::uci_loop();
}