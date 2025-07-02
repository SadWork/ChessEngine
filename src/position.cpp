#pragma once
#include "position.hpp"


Position::Position(std::array<uint16_t, static_cast<uint16_t>(Map::CNT_SQUARES)> &board,
            uint16_t features, uint16_t rule50cnt, uint16_t enpassant_target_square)
    : features{features}, rule50cnt{rule50cnt}, enpassant_target_square{enpassant_target_square}, end_pieces_list{0}, moves(), state_history(), king_sq{static_cast<uint16_t>(Map::CNT_SQUARES), static_cast<uint16_t>(Map::CNT_SQUARES)}
{
    this->board.fill(static_cast<uint16_t>(Map::CNT_SQUARES));
    pieces_list.fill(Piece::none());

    for (std::size_t i = 0; i < board.size(); ++i)
    {
        if (board[i] != static_cast<uint16_t>(PieceType::EMPTY))
        {
            pieces_list[end_pieces_list] = {board[i],
                                            static_cast<uint16_t>(i)};
            this->board[i] = end_pieces_list++;
            if(FEN::get_piece_type(board[i]) == PieceType::KING){
                king_sq[static_cast<size_t>(FEN::get_piece_color(board[i]))] = static_cast<uint16_t>(i);
            }
        }
    }
}

Position::Position(uint16_t features, uint16_t rule50cnt)
    : features(features), rule50cnt(rule50cnt), enpassant_target_square(static_cast<uint16_t>(Map::CNT_SQUARES)), end_pieces_list(0), moves(), state_history(), king_sq{static_cast<uint16_t>(Map::CNT_SQUARES), static_cast<uint16_t>(Map::CNT_SQUARES)}
{
    pieces_list.fill(Piece::none());
    board.fill(static_cast<uint16_t>(Map::CNT_SQUARES));
}

void Position::set_from_fen(std::string_view fen_view)
{
    moves.clear();
    state_history.clear();

    pieces_list.fill(Piece::none());
    board.fill(static_cast<uint16_t>(Map::CNT_SQUARES));
    features = 0;
    rule50cnt = 0;

    auto current = fen_view.begin();
    auto end = fen_view.end();

    auto read_part = [&]() -> std::string_view
    {
        if (current == end)
        {
            throw std::runtime_error("Invalid FEN string: unexpected end of string.");
        }
        while (current != end && *current == ' ')
        {
            ++current;
        }
        auto start_part = current;
        while (current != end && *current != ' ')
        {
            ++current;
        }
        return std::string_view(start_part, std::distance(start_part, current));
    };

    std::string_view piece_placement_part = read_part();
    int rank = static_cast<uint16_t>(Map::HEIGHT) - 1;
    int file = 0;
    end_pieces_list = 0;
    for (char c : piece_placement_part)
    {
        if (c == '/')
        {
            if (file != static_cast<int>(Map::WIDTH))
            {
                throw std::runtime_error("Invalid FEN string: rank description ended prematurely.");
            }
            rank--;
            file = 0;
            if (rank < 0)
            {
                throw std::runtime_error("Invalid FEN string: too many ranks.");
            }
        }
        else if (c >= '1' && c <= '8')
        {
            int empty_count = c - '0';
            if (file + empty_count > static_cast<int>(Map::WIDTH))
            {
                throw std::runtime_error("Invalid FEN string: invalid empty square count in rank.");
            }
            file += empty_count;
        }
        else
        {
            if (file >= static_cast<int>(Map::WIDTH))
            {
                throw std::runtime_error("Invalid FEN string: too many board/empty squares in rank.");
            }
            uint16_t piece = FEN::fen_char_to_piece(c);
            if (piece == static_cast<uint16_t>(PieceType::EMPTY))
            {
                throw std::runtime_error("Invalid FEN string: unrecognized character in piece placement.");
            }

            uint16_t position = rank * static_cast<uint16_t>(Map::WIDTH) + file;
            
            if(FEN::get_piece_type(piece) == PieceType::KING){
                king_sq[static_cast<size_t>(FEN::get_piece_color(piece))] = position;
            }

            pieces_list[end_pieces_list] = {piece, position};
            board[position] = end_pieces_list++;
            file++;
        }
    }
    if (rank != 0 || file != static_cast<uint16_t>(Map::WIDTH))
    {
        throw std::runtime_error("Invalid FEN string: board description incomplete or overflown.");
    }

    std::string_view side_part = read_part();
    if (side_part == "w")
    {
        features |= static_cast<uint16_t>(Color::WHITE) << static_cast<uint16_t>(Map::LOG_BIT_SIDE_TO_MOVE);
    }
    else if (side_part == "b")
    {
        features |= static_cast<uint16_t>(Color::BLACK) << static_cast<uint16_t>(Map::LOG_BIT_SIDE_TO_MOVE);
    }
    else
    {
        throw std::runtime_error("Invalid FEN string: invalid side to move.");
    }

    std::string_view castling_part = read_part();

    features |= static_cast<uint16_t>(Map::BIT_NO_CASTLE_WK) |
                static_cast<uint16_t>(Map::BIT_NO_CASTLE_WQ) |
                static_cast<uint16_t>(Map::BIT_NO_CASTLE_BK) |
                static_cast<uint16_t>(Map::BIT_NO_CASTLE_BQ);

    if (castling_part != "-")
    {
        for (char c : castling_part)
        {
            switch (c)
            {
            case 'K':
                features &= ~static_cast<uint16_t>(Map::BIT_NO_CASTLE_WK);
                break;
            case 'Q':
                features &= ~static_cast<uint16_t>(Map::BIT_NO_CASTLE_WQ);
                break;
            case 'k':
                features &= ~static_cast<uint16_t>(Map::BIT_NO_CASTLE_BK);
                break;
            case 'q':
                features &= ~static_cast<uint16_t>(Map::BIT_NO_CASTLE_BQ);
                break;
            default:
                throw std::runtime_error("Invalid FEN string: invalid castling rights character.");
            }
        }
    }

    std::string_view enpassant_part = read_part();
    if (enpassant_part != "-")
    {
        int ep_idx = FEN::square_to_index(enpassant_part);
        if (ep_idx == -1)
        {
            throw std::runtime_error("Invalid FEN string: invalid en passant square.");
        }
        enpassant_target_square = static_cast<uint16_t>(ep_idx);
    }
    else
    {
        enpassant_target_square = static_cast<uint16_t>(Map::CNT_SQUARES);
    }

    if (current != end)
    { // Проверяем, есть ли еще части
        std::string_view rule50_part = read_part();
        int value = 0;
        auto result = std::from_chars(rule50_part.data(), rule50_part.data() + rule50_part.size(), value);
        if (result.ec == std::errc() && result.ptr == rule50_part.data() + rule50_part.size() && value >= 0 && value <= 255)
        {
            rule50cnt = static_cast<uint16_t>(value);
        }
        else
        {
            throw std::runtime_error("Invalid FEN string: invalid halfmove clock value.");
        }
    }
    else
    {
        rule50cnt = 0;
    }

    if (current != end)
    { // Проверяем, есть ли еще часть
        std::string_view fullmove_part = read_part();
        int value = 0;
        auto result = std::from_chars(fullmove_part.data(), fullmove_part.data() + fullmove_part.size(), value);
        if (result.ec != std::errc() || result.ptr != fullmove_part.data() + fullmove_part.size() || value < 1)
        {
            throw std::runtime_error("Invalid FEN string: invalid fullmove number value.");
        }
        // Номер хода не сохраняем, только валидируем
    }

    if (current != end)
    {                                                 // Если после последней части что-то осталось...
        std::string_view trailing_part = read_part(); // Пытаемся прочитать еще одну часть
        if (!trailing_part.empty())
        {
            throw std::runtime_error("Invalid FEN string: unexpected characters after fullmove number.");
        }
    }
}

void  Position::do_move(Move m)
{
    uint16_t side_to_move_bit = (features >> static_cast<uint16_t>(Map::LOG_BIT_SIDE_TO_MOVE)) & 0x1;

    const uint16_t source_sq = m.source();
    const uint16_t dest_sq = m.dest();

    uint16_t moved_piece_list_idx = board[source_sq];
    uint16_t moved_piece_code = pieces_list[moved_piece_list_idx].type;

    PieceType moved_piece_type = FEN::get_piece_type(moved_piece_code);

    uint16_t captured_piece_list_idx = board[dest_sq];
    uint16_t captured_piece_code = pieces_list[captured_piece_list_idx].type;
    uint16_t captured_piece_sq = dest_sq;

    PieceType captured_piece_type = FEN::get_piece_type(captured_piece_code);
    // if captured_piece_list_idx == Map::CNT_SQUARES it's still in the range of array

    const MoveType m_type = m.type();
    bool is_enpassant{m_type == MoveType::EN_PASSANT};
    bool is_captured{FEN::get_piece_type(captured_piece_code) != PieceType::EMPTY};
    bool is_pawn_move{moved_piece_type == PieceType::PAWN};
    bool is_pawn_long_move{is_pawn_move and ((source_sq > dest_sq ? source_sq - dest_sq : dest_sq - source_sq) == 2 * static_cast<uint16_t>(Map::WIDTH))};

    if (moved_piece_type == PieceType::KING)
    {
        king_sq[side_to_move_bit] = dest_sq;
    }

    if (is_enpassant)
    {
        captured_piece_sq = static_cast<int>(dest_sq) +
                            (side_to_move_bit == static_cast<uint16_t>(Color::WHITE) ? -static_cast<int>(Map::WIDTH) : static_cast<int>(Map::WIDTH));
        captured_piece_list_idx = board[captured_piece_sq];
        captured_piece_code = pieces_list[captured_piece_list_idx].type;
        
        // after that remove as it's normal move
        is_captured = true;
    }
    else if (m_type == MoveType::PROMOTION)
    {
        moved_piece_code = static_cast<uint16_t>(m.promotion_piece()) | (side_to_move_bit << static_cast<uint16_t>(Color::LOG_BIT_COLOR));
        pieces_list[moved_piece_list_idx].type = moved_piece_code;
    }
    else if (m_type == MoveType::CASTLING)
    {
        bool is_short = dest_sq > source_sq;
        // if short
        // h1/h8 -> f1/f8.
        // if long
        // a1/a8 -> d1/d8.

        uint16_t rook_from_sq = (is_short) ? dest_sq + 1 : dest_sq - 2;
        uint16_t rook_to_sq = (is_short) ? dest_sq - 1 : dest_sq + 1;

        uint16_t rook_list_idx = board[rook_from_sq];
        pieces_list[rook_list_idx].position = rook_to_sq;
        board[rook_to_sq] = rook_list_idx;
        board[rook_from_sq] = static_cast<uint16_t>(Map::CNT_SQUARES);
    }

    if (is_captured or is_enpassant)
    { // remove captured piece from the board
        --end_pieces_list;
        board[pieces_list[end_pieces_list].position] = captured_piece_list_idx; // set new idx on the board for last piece in the list
        pieces_list[captured_piece_list_idx] = pieces_list[end_pieces_list];    // move last piece info into new idx in the list
        pieces_list[end_pieces_list] = Piece::none();                           // remove caputred piece from the list
        board[captured_piece_sq] = static_cast<uint16_t>(Map::CNT_SQUARES); // remove captured piece from the board

        moved_piece_list_idx = moved_piece_list_idx >= end_pieces_list ? // if yes than it was swaped with captured_piece
                                    captured_piece_list_idx              // overwise it wasn't
                                                                        : moved_piece_list_idx;
    }

    board[source_sq] = static_cast<uint16_t>(Map::CNT_SQUARES);
    board[dest_sq] = moved_piece_list_idx;
    pieces_list[moved_piece_list_idx].position = dest_sq;

    state_history.emplace_back(features, rule50cnt, enpassant_target_square, captured_piece_code, captured_piece_sq);
    moves.emplace_back(std::move(m));

    uint16_t new_enpassant_target = static_cast<int>(dest_sq) +
                                    (side_to_move_bit == static_cast<uint16_t>(Color::WHITE) ? -static_cast<int>(Map::WIDTH) : static_cast<int>(Map::WIDTH));
    enpassant_target_square = (is_pawn_long_move) ? new_enpassant_target : static_cast<uint16_t>(Map::CNT_SQUARES);

    features |= (moved_piece_type == PieceType::KING) ? (side_to_move_bit == static_cast<uint16_t>(Color::BLACK)) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BK) |
                                                            (side_to_move_bit == static_cast<uint16_t>(Color::BLACK)) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BQ) |
                                                            (side_to_move_bit == static_cast<uint16_t>(Color::WHITE)) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WK) |
                                                            (side_to_move_bit == static_cast<uint16_t>(Color::WHITE)) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WQ)
                                                        : 0;
    if(moved_piece_type == PieceType::ROOK){
        features |= (source_sq == FEN::square_to_index("h1")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WK);
        features |= (source_sq == FEN::square_to_index("a1")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WQ);
        features |= (source_sq == FEN::square_to_index("h8")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BK);
        features |= (source_sq == FEN::square_to_index("a8")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BQ);
    }
    if(captured_piece_type == PieceType::ROOK){
        features |= (dest_sq == FEN::square_to_index("h1")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WK);
        features |= (dest_sq == FEN::square_to_index("a1")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_WQ);
        features |= (dest_sq == FEN::square_to_index("h8")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BK);
        features |= (dest_sq == FEN::square_to_index("a8")) << static_cast<uint16_t>(Map::LOG_BIT_NO_CASTLE_BQ);
    }

    features ^= static_cast<uint16_t>(Map::BIT_SIDE_TO_MOVE);
    rule50cnt = (is_captured or is_pawn_move) ? 0 : rule50cnt + 1;
}

void Position::undo_move()
{
    StateInfo &st = state_history.back();
    features = st.features;
    rule50cnt = st.rule50cnt;
    enpassant_target_square = st.enpassant_target_square;

    uint16_t captured_piece_code = st.captured_piece_code; // Что взяли
    uint16_t captured_piece_sq = st.captured_piece_sq;     // Где оно стояло

    Move &m = moves.back();
    const uint16_t source_sq = m.source();
    const uint16_t dest_sq = m.dest();
    const MoveType m_type = m.type();
    uint16_t side_to_move_bit = (features >> static_cast<uint16_t>(Map::LOG_BIT_SIDE_TO_MOVE)) & 0x1;

    uint16_t moved_piece_list_idx = board[dest_sq];           // Находим индекс фигуры, которая сделала ход (она сейчас на dest_sq)
    board[source_sq] = moved_piece_list_idx;                  // Ставим фигуру обратно на source_sq
    board[dest_sq] = static_cast<uint16_t>(Map::CNT_SQUARES); // Очищаем dest_sq
    pieces_list[moved_piece_list_idx].position = source_sq;   // Обновляем позицию в списке

    // Отменяем превращение (если было)
    if (m_type == MoveType::PROMOTION)
    {
        // Возвращаем тип пешки
        pieces_list[moved_piece_list_idx].type = static_cast<uint16_t>(PieceType::PAWN) | (side_to_move_bit << static_cast<uint16_t>(Color::LOG_BIT_COLOR));
    }

    // 4. Отменяем рокировку (если была)
    if (m_type == MoveType::CASTLING)
    {
        bool is_short = dest_sq > source_sq;
        // Позиции ладьи ПОСЛЕ рокировки
        uint16_t rook_current_sq = (is_short) ? dest_sq - 1 : dest_sq + 1;
        // Позиции ладьи ДО рокировки
        uint16_t rook_original_sq = (is_short) ? dest_sq + 1 : dest_sq - 2;

        uint16_t rook_list_idx = board[rook_current_sq];                  // Находим индекс ладьи
        pieces_list[rook_list_idx].position = rook_original_sq;           // Возвращаем позицию ладьи
        board[rook_original_sq] = rook_list_idx;                          // Ставим ладью на доску
        board[rook_current_sq] = static_cast<uint16_t>(Map::CNT_SQUARES); // Убираем с промежуточного поля
    }

    PieceType moved_piece_type = FEN::get_piece_type(pieces_list[moved_piece_list_idx].type); // Не считая promotion, мы не трогали тип фигуры
    if(moved_piece_type == PieceType::KING){
        king_sq[side_to_move_bit] = source_sq;
    }

    if (captured_piece_code != static_cast<uint16_t>(PieceType::EMPTY))
    {
        // Увеличиваем счетчик фигур
        uint16_t new_captured_list_idx = end_pieces_list++;

        // Помещаем взятую фигуру в конец списка
        pieces_list[new_captured_list_idx] = {captured_piece_code, captured_piece_sq};

        // Ставим взятую фигуру обратно на доску
        board[captured_piece_sq] = new_captured_list_idx;
    }

    moves.pop_back();
    state_history.pop_back();
}