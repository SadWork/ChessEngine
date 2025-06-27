#include "movegen.hpp"

namespace MoveGen
{
    // Смещения для коня
    constexpr int KNIGHT_MOVES[] = {NORTH + NORTH_EAST, NORTH + NORTH_WEST, EAST + NORTH_EAST, EAST + SOUTH_EAST, SOUTH + SOUTH_EAST, SOUTH + SOUTH_WEST, WEST + SOUTH_WEST, WEST + NORTH_WEST}; 
    // 17, 15, 10, -6, -15, -17, -10, 6

    // Смещения для короля
    constexpr int KING_MOVES[] = {NORTH, NORTH_EAST, EAST, SOUTH_EAST, SOUTH, SOUTH_WEST, WEST, NORTH_WEST}; 
    // 8, 9, 1, -7, -8, -9, -1, 7

    // Направления для ладьи
    constexpr int ROOK_DIRECTIONS[] = {NORTH, EAST, SOUTH, WEST};
    // Направления для слона
    constexpr int BISHOP_DIRECTIONS[] = {NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};

    void generate_moves(const Position &pos, std::vector<Move> &move_list)
    {
        move_list.clear();
        move_list.reserve(218);
        
        Color side_to_move = static_cast<Color>(Position::get_side_to_move(pos));
        for (uint16_t i = 0; i < pos.end_pieces_list; ++i)
        {
            const Piece& p = pos.pieces_list[i];
            Color piece_color = FEN::get_piece_color(p.type);

            if (piece_color != side_to_move) continue;

            PieceType piece_type = FEN::get_piece_type(p.type);
            uint16_t from_sq = p.position;

            switch (piece_type)
            {
            case PieceType::PAWN:
                generate_pawn_moves(pos, from_sq, move_list);
                break;
            case PieceType::KNIGHT:
                break;
            case PieceType::BISHOP:
                break;
            case PieceType::ROOK:
                break;
            case PieceType::QUEEN:
                break;
            case PieceType::KING:
 
                // TODO: Добавить генерацию рокировок
                break;
            case PieceType::EMPTY:
            default:
                break;
            }
        }
    }

    void generate_pawn_moves(const Position &pos, uint16_t from_sq, std::vector<Move> &move_list) {
        Color side_to_move = static_cast<Color>(Position::get_side_to_move(pos));

        int push_once = side_to_move == Color::WHITE ? NORTH : SOUTH;
        
        int push_once_dest = from_sq + push_once; // always is valid if pawn is not on the first or last rank
        int push_twice_dest = push_once_dest + push_once; // always is valid if is_promotion == false
        bool is_promotion = push_twice_dest < 0 or push_twice_dest >= static_cast<int>(Map::CNT_SQUARES);

        int push_twice_inv_dest = from_sq - 2*push_once;
        bool is_start_rank =  push_twice_inv_dest < 0 or push_twice_inv_dest >= static_cast<int>(Map::CNT_SQUARES);

        // No capture
        if(pos.board[push_once_dest] == static_cast<uint16_t>(Map::CNT_SQUARES)){
            if(is_promotion){
                move_list.emplace_back(from_sq, push_once_dest, PieceType::QUEEN);
                move_list.emplace_back(from_sq, push_once_dest, PieceType::ROOK);
                move_list.emplace_back(from_sq, push_once_dest, PieceType::BISHOP);
                move_list.emplace_back(from_sq, push_once_dest, PieceType::KNIGHT);
            }else{
                move_list.emplace_back(from_sq, push_once_dest);
                if(is_start_rank){
                    move_list.emplace_back(from_sq, push_twice_dest);
                }
            }
        }

        // Captures
        int capture_dirs[] = {(side_to_move == Color::WHITE) ? NORTH_WEST : SOUTH_WEST, (side_to_move == Color::WHITE) ? NORTH_EAST : SOUTH_EAST}; 

        for(int dir : capture_dirs){
            int dest_sq = from_sq + dir;

            if(dest_sq < 0 or dest_sq >= static_cast<int>(Map::CNT_SQUARES) or FEN::dist(from_sq, dest_sq) > 2){
                continue;
            }

            uint16_t captured_idx = pos.board[dest_sq];
            if (captured_idx != static_cast<uint16_t>(Map::CNT_SQUARES) && FEN::get_piece_color(pos.pieces_list[captured_idx].type) != side_to_move){
                if (is_promotion)
                {
                    move_list.emplace_back(from_sq, dest_sq, PieceType::QUEEN);
                    move_list.emplace_back(from_sq, dest_sq, PieceType::ROOK);
                    move_list.emplace_back(from_sq, dest_sq, PieceType::BISHOP);
                    move_list.emplace_back(from_sq, dest_sq, PieceType::KNIGHT);
                }
                else
                {
                    move_list.emplace_back(from_sq, dest_sq);
                }
            }

            if (dest_sq == pos.enpassant_target_square)
            {
                move_list.emplace_back(from_sq, dest_sq, MoveType::EN_PASSANT);
            }
        }    
           
    }
    
}