#include "movegen.hpp"

namespace MoveGen
{
    // Смещения для коня
    constexpr int KNIGHT_DIRECTIONS[] = {NORTH + NORTH_EAST, NORTH + NORTH_WEST, EAST + NORTH_EAST, EAST + SOUTH_EAST, SOUTH + SOUTH_EAST, SOUTH + SOUTH_WEST, WEST + SOUTH_WEST, WEST + NORTH_WEST}; 
    // 17, 15, 10, -6, -15, -17, -10, 6

    constexpr size_t CASTLE_N = 2, COLOR_N = 2;

    constexpr uint16_t CASTLING_RIGHTS_MASKS[COLOR_N][CASTLE_N] = {
        {static_cast<uint16_t>(Map::BIT_NO_CASTLE_WK), static_cast<uint16_t>(Map::BIT_NO_CASTLE_WQ)}, 
        static_cast<uint16_t>(Map::BIT_NO_CASTLE_BK), static_cast<uint16_t>(Map::BIT_NO_CASTLE_BQ)
    };
    constexpr int CASTLING_DIRECTION[COLOR_N][CASTLE_N] = {
        {EAST, WEST}, 
        {EAST, WEST}
    };
    constexpr uint16_t ROOK_CASTLING_SQ[COLOR_N][CASTLE_N] = {
        {FEN::square_to_index("h1"), FEN::square_to_index("a1")}, 
        {FEN::square_to_index("h8"), FEN::square_to_index("a8")}
    };
    
    // Направления для короля
    constexpr int KING_DIRECTIONS[] = {NORTH, EAST, SOUTH, WEST, NORTH_EAST, NORTH_WEST, SOUTH_EAST, SOUTH_WEST};
    // Направления для ладьи
    constexpr int const *ROOK_DIRECTIONS = KING_DIRECTIONS;
    // Направления для слона
    constexpr int const *BISHOP_DIRECTIONS = KING_DIRECTIONS+sizeof(KING_DIRECTIONS)/sizeof(KING_DIRECTIONS[0])/2;

    

    void generate_attacks(const Position &pos, Color side_to_move, AttacksArray &attacks_list)
    {
        attacks_list.size.fill(0);

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
                generate_pawn_attacks(pos, side_to_move, from_sq, attacks_list);
                break;
            case PieceType::KNIGHT:
                generate_leaping_attacks(pos, side_to_move, from_sq, KNIGHT_DIRECTIONS, 8, attacks_list);
                break;
            case PieceType::BISHOP:
                generate_sliding_attacks(pos, side_to_move, from_sq, BISHOP_DIRECTIONS, 4, attacks_list);
                break;
            case PieceType::ROOK:
                generate_sliding_attacks(pos, side_to_move, from_sq, ROOK_DIRECTIONS, 4, attacks_list);
                break;
            case PieceType::QUEEN:
                generate_sliding_attacks(pos, side_to_move, from_sq, KING_DIRECTIONS, 8, attacks_list);
                break;
            case PieceType::KING:
                generate_leaping_attacks(pos, side_to_move, from_sq, KING_DIRECTIONS, 8, attacks_list);
                break;
            case PieceType::EMPTY:
            default:
                break;
            }
        }
    }

    void generate_moves(Position &pos, AttacksArray &attacks_list, std::vector<MoveInfo> &move_list)
    {
        // move_list.clear();
        // move_list.reserve(218);

        Color side_to_move = static_cast<Color>(Position::get_side_to_move(pos));
        uint16_t color_bit = static_cast<uint16_t>(side_to_move) << static_cast<uint16_t>(Color::LOG_BIT_COLOR);

        for(size_t i = 0; i< attacks_list.size.size(); ++i){
            // TODO : Remove checking promotion for 2-7 ranks
            size_t size = attacks_list.size[i];
            for(size_t j = 0; j < size; ++j){
                Move m = attacks_list.sq[i*static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ)+j];
                
                uint16_t source_sq = m.source();
                uint16_t moved_idx = pos.board[m.source()];
                PieceType moved_piece = FEN::get_piece_type(pos.pieces_list[moved_idx].type);

                uint16_t dest_sq = m.dest();
                uint16_t captured_idx = pos.board[m.dest()];
                Piece caputred_piece = pos.pieces_list[captured_idx];
                uint16_t captured_color_bit = caputred_piece.type & static_cast<uint16_t>(Color::BIT_COLOR);
                
                if(color_bit == captured_color_bit and captured_idx != static_cast<uint16_t>(Map::CNT_SQUARES)){
                    continue;
                }
                if(moved_piece == PieceType::PAWN and (dest_sq != pos.enpassant_target_square and captured_idx == static_cast<uint16_t>(Map::CNT_SQUARES)))
                {
                    continue;
                }

                auto next_attacks = std::make_shared<AttacksArray>();
                if(is_legal(m, pos, attacks_list, *next_attacks)){
                    if(moved_piece == PieceType::PAWN){
                        generate_pawn_promotions(m, side_to_move, move_list, next_attacks);
                        continue;
                    }
                    move_list.emplace_back(m,next_attacks);
                }
            }
        }

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
                {
                    PawnQuiteMoves pawn_list;
                    pawn_list.fill(Move::none());
                    generate_pawn_moves(pos,side_to_move, from_sq, pawn_list);
                    
                    size_t size = 0;
                    while(pawn_list[size] != Move::none()){
                        auto next_attacks = std::make_shared<AttacksArray>();
                        if(is_legal(pawn_list[size], pos, attacks_list, *next_attacks)){
                            generate_pawn_promotions(pawn_list[size], side_to_move, move_list, next_attacks);
                        }
                        ++size;
                    }
                }
                break;
            case PieceType::KING:
                generate_castling_moves(pos, side_to_move, attacks_list, move_list);            
                break;
            case PieceType::EMPTY:
            default:
                break;
            }
        }
    }

    void generate_pawn_moves(const Position &pos, Color side_to_move, uint16_t from_sq, PawnQuiteMoves &move_list) {
        size_t size = 0;

        int push_once = side_to_move == Color::WHITE ? NORTH : SOUTH;
        
        int push_once_dest = from_sq + push_once; // always is valid if pawn is not on the first or last rank
        int push_twice_dest = push_once_dest + push_once; // always is valid if is_promotion == false

        int push_twice_inv_dest = from_sq - 2*push_once;
        bool is_start_rank =  push_twice_inv_dest < 0 or push_twice_inv_dest >= static_cast<int>(Map::CNT_SQUARES);

        // No capture
        if(pos.board[push_once_dest] == static_cast<uint16_t>(Map::CNT_SQUARES)){
            move_list[size++] = Move(from_sq, push_once_dest);
            if (is_start_rank and pos.board[push_twice_dest] == static_cast<uint16_t>(Map::CNT_SQUARES)){
                move_list[size++] = Move(from_sq, push_twice_dest);
            }
        }    
    }
    
    void generate_pawn_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, AttacksArray &attacks_list)
    {

        // Captures
        int capture_dirs[] = {(side_to_move == Color::WHITE) ? NORTH_WEST : SOUTH_WEST, (side_to_move == Color::WHITE) ? NORTH_EAST : SOUTH_EAST}; 

        for(int dir : capture_dirs){
            int dest_sq = from_sq + dir;

            if(dest_sq < 0 or dest_sq >= static_cast<int>(Map::CNT_SQUARES) or FEN::dist(from_sq, dest_sq) > 2){
                continue;
            }

            uint16_t captured_idx = pos.board[dest_sq];
            size_t offset = attacks_list.size[dest_sq]++;
            
            if(dest_sq == pos.enpassant_target_square){
                attacks_list.sq[dest_sq*static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ) + offset] = Move(from_sq, dest_sq, MoveType::EN_PASSANT);
            }else{
                attacks_list.sq[dest_sq*static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ) + offset] = Move(from_sq, dest_sq);
            }
        }    
    }

    void generate_pawn_promotions(Move move, Color side_to_move, std::vector<MoveInfo> &move_list, std::shared_ptr<AttacksArray> attacksArray)
    {
        int push_twice_dest = move.dest() + (side_to_move == Color::WHITE ? NORTH : SOUTH);   
        bool is_promotion = push_twice_dest < 0 or push_twice_dest >= static_cast<int>(Map::CNT_SQUARES);

        if(is_promotion){
            move.set_promotion(PieceType::QUEEN);
            move_list.emplace_back(move, attacksArray);
            move.set_promotion(PieceType::ROOK);
            move_list.emplace_back(move, attacksArray);
            move.set_promotion(PieceType::BISHOP);
            move_list.emplace_back(move, attacksArray);
            move.set_promotion(PieceType::KNIGHT);
            move_list.emplace_back(move, attacksArray);
        }
        else{
            move_list.emplace_back(move, attacksArray);
        }
    }

    void generate_sliding_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, const int *directions, int num_directions, AttacksArray &attacks_list)
    {

        for (int i = 0; i < num_directions; ++i)
        {
            int direction = directions[i];
            for (int to_sq = from_sq + direction;
                 to_sq >= 0 && to_sq < static_cast<int>(Map::CNT_SQUARES);
                 to_sq += direction)
            {
                int dist = FEN::dist(to_sq - direction, to_sq);
                if (dist > 2) break;

                uint16_t captured_piece_idx = pos.board[to_sq];
                size_t offset = attacks_list.size[to_sq]++;
                attacks_list.sq[to_sq*static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ)+offset] = Move(from_sq, to_sq);
                if (captured_piece_idx != static_cast<uint16_t>(Map::CNT_SQUARES))
                {
                    break;
                }
            }
        }
    }

    void generate_leaping_attacks(const Position &pos, Color side_to_move, uint16_t from_sq, const int *directions, int num_directions, AttacksArray &attacks_list)
    {
        for (int i = 0; i < num_directions; ++i)
        {
            int to_sq = from_sq + directions[i];

            if (to_sq < 0 || to_sq >= static_cast<int>(Map::CNT_SQUARES) || FEN::dist(from_sq, to_sq) > 3) continue;

            uint16_t captured_piece_idx = pos.board[to_sq];
            // Если поле пустое или занято фигурой противника
            if (captured_piece_idx == static_cast<uint16_t>(Map::CNT_SQUARES) || FEN::get_piece_color(pos.pieces_list[captured_piece_idx].type) != side_to_move)
            {
                size_t offset = attacks_list.size[to_sq]++;
                attacks_list.sq[to_sq*static_cast<size_t>(Map::MAX_ATTACKS_PER_SQ)+offset] = Move(from_sq, to_sq);
            }
        }
    }


    void generate_castling_moves(Position &pos, Color side_to_move, const AttacksArray &attacks_list, std::vector<MoveInfo> &move_list) {
        size_t side = static_cast<size_t>(side_to_move);

        for(size_t i = 0; i < CASTLE_N; ++i) {
            if(!(pos.features & CASTLING_RIGHTS_MASKS[side][i])) {
                int  dir = CASTLING_DIRECTION[side][i];
                uint16_t king_sq = pos.king_sq[side];
                
                bool is_free = true;
                uint16_t target_sq = ROOK_CASTLING_SQ[side][i];
                for(uint16_t i = king_sq+dir; i != target_sq; i += dir) {
                    if(pos.board[i] != static_cast<uint16_t>(Map::CNT_SQUARES)) {
                        is_free = false;
                        break;
                    }
                }
                
                if(is_free){
                    Move m = Move(king_sq, king_sq + 2*dir, MoveType::CASTLING);
                    auto next_attacks = std::make_shared<AttacksArray>();

                    if(is_legal(m, pos, attacks_list, *next_attacks) and !next_attacks->size[king_sq] and !next_attacks->size[king_sq + dir]) {
                        move_list.emplace_back(m, next_attacks);
                    }
                }
            }
        }
    }

    bool is_legal(const Move move, Position &pos, const AttacksArray &attacks_list, AttacksArray &next_attacks) {
        uint16_t side_bit_before_move = static_cast<uint16_t>(Position::get_side_to_move(pos));
        bool legal = true;
        // if(std::string(FEN::index_to_square(move.source()))+ std::string(FEN::index_to_square(move.dest())) == "f4g3"){
        //     std::println("{}", pos);
        //     std::println("{:l}", pos);
        // }
        pos.do_move(move);
        // if(std::string(FEN::index_to_square(move.source()))+ std::string(FEN::index_to_square(move.dest())) == "f4g3"){
        //     std::println("{}", pos);
        //     std::println("{:l}", pos);
        //     std::println("{}", pos.board[FEN::square_to_index("g4")]);
        // }
        Color side_to_move = static_cast<Color>(Position::get_side_to_move(pos));
        generate_attacks(pos, side_to_move,  next_attacks);

        if(next_attacks.size[pos.king_sq[side_bit_before_move]]){
            legal = false;
        }
        pos.undo_move();
        return legal;
    }
}