#pragma once
#include "types.h"

namespace FEN
{
    // Получить тип фигуры из закодированного значения
    constexpr inline PieceType get_piece_type(uint16_t piece_code)
    {
        return static_cast<PieceType>(static_cast<uint16_t>(piece_code) & static_cast<uint16_t>(PieceType::MASK)); // Младшие 3 бита - тип
    }

    // Получить цвет фигуры из закодированного значения
    constexpr inline Color get_piece_color(uint16_t piece_code)
    {
        return static_cast<Color>((static_cast<uint16_t>(piece_code) & static_cast<uint16_t>(Color::BIT_COLOR)) >> static_cast<uint16_t>(Color::LOG_BIT_COLOR));
    }

    // Закодировать фигуру
    constexpr inline uint16_t make_piece_code(Color color, PieceType type)
    {
        return (static_cast<uint16_t>(color) << static_cast<uint16_t>(Color::LOG_BIT_COLOR)) | static_cast<uint16_t>(type);
    }

    constexpr inline uint16_t fen_char_to_piece(char c)
    {
        switch (c)
        {
        case 'P':
            return make_piece_code(Color::WHITE, PieceType::PAWN);
        case 'N':
            return make_piece_code(Color::WHITE, PieceType::KNIGHT);
        case 'B':
            return make_piece_code(Color::WHITE, PieceType::BISHOP);
        case 'R':
            return make_piece_code(Color::WHITE, PieceType::ROOK);
        case 'Q':
            return make_piece_code(Color::WHITE, PieceType::QUEEN);
        case 'K':
            return make_piece_code(Color::WHITE, PieceType::KING);
        case 'p':
            return make_piece_code(Color::BLACK, PieceType::PAWN);
        case 'n':
            return make_piece_code(Color::BLACK, PieceType::KNIGHT);
        case 'b':
            return make_piece_code(Color::BLACK, PieceType::BISHOP);
        case 'r':
            return make_piece_code(Color::BLACK, PieceType::ROOK);
        case 'q':
            return make_piece_code(Color::BLACK, PieceType::QUEEN);
        case 'k':
            return make_piece_code(Color::BLACK, PieceType::KING);
        default:
            return static_cast<uint16_t>(PieceType::EMPTY);
        }
    }

    constexpr inline int square_to_index(std::string_view sq_str)
    {
        char file_char = sq_str[0];
        char rank_char = sq_str[1];

        int file = file_char - 'a';
        int rank = rank_char - '1';
        return rank * static_cast<int>(Map::WIDTH) + file;
    }

    constexpr inline uint16_t dist(uint16_t from, uint16_t to)
    {
        uint16_t x1{from & 0x7};
        uint16_t y1{from >> 3};
        uint16_t x2{to & 0x7};
        uint16_t y2{from >> 3};
        uint16_t delta_x = x1 > x2 ? x1 - x2 : x2 - x1;
        uint16_t delta_y = y1 > y2 ? y1 - y2 : y2 - y1;
        return delta_x + delta_y;
    }

    constexpr inline char piece_to_fen_char(uint16_t p)
    {
        PieceType piece_type = get_piece_type(p);
        Color color = get_piece_color(p);

        char c = '?';
        switch (piece_type)
        {
        case PieceType::EMPTY:
            c = '.';
            break;
        case PieceType::PAWN:
            c = 'P';
            break;
        case PieceType::KNIGHT:
            c = 'N';
            break;
        case PieceType::BISHOP:
            c = 'B';
            break;
        case PieceType::ROOK:
            c = 'R';
            break;
        case PieceType::QUEEN:
            c = 'Q';
            break;
        case PieceType::KING:
            c = 'K';
            break;
        }

        return color == Color::BLACK ? std::tolower(c) : c;
    }

    // Преобразует индекс (0-63) в строку поля ("a1"-"h8")
    inline std::string index_to_square(int index)
    {
        if (index < 0 || index >= static_cast<int>(Map::CNT_SQUARES))
            return "??";
        char file = 'a' + (index % static_cast<int>(Map::WIDTH));
        char rank = '1' + (index / static_cast<int>(Map::WIDTH));
        return {file, rank};
    }
};

struct Piece
{
    static consteval Piece none()
    {
        return {static_cast<uint16_t>(PieceType::EMPTY), static_cast<uint16_t>(Map::CNT_SQUARES)};
    }
    uint16_t type, position;
};

enum class MoveType : std::uint16_t
{
    NORMAL = 0 << 14,
    PROMOTION = 1 << 14,
    EN_PASSANT = 2 << 14,
    CASTLING = 3 << 14,
    MASK = 3 << 14,
    PROMOTION_MASK = 3 << 12,
};

struct Move
{
    constexpr explicit Move(std::uint16_t d = 0) : data(d) {}
    constexpr Move(uint16_t from, uint16_t to, MoveType mt = MoveType::NORMAL)
        : data(static_cast<std::uint16_t>(mt) | (std::uint16_t(from) << 6) | to)
    {
    }
    constexpr Move(uint16_t from, uint16_t to, PieceType pt)
        : data(static_cast<std::uint16_t>(MoveType::PROMOTION) | ((static_cast<uint16_t>(pt) - static_cast<uint16_t>(PieceType::KNIGHT)) << 12) | (from << 6) | to)
    {
    }

    static consteval Move null()
    {
        return Move(static_cast<uint16_t>(Map::CNT_SQUARES) | (static_cast<uint16_t>(Map::CNT_SQUARES) << 6));
    }
    static consteval Move none()
    {
        return Move(0);
    }

    std::uint16_t data;
    constexpr inline uint16_t dest() const
    {
        return data & 0x3F;
    }
    constexpr inline uint16_t source() const
    {
        return (data >> 6) & 0x3F;
    }
    constexpr inline PieceType promotion_piece()
    {
        return PieceType(static_cast<uint16_t>(PieceType::KNIGHT) + ((data & static_cast<uint16_t>(MoveType::PROMOTION_MASK)) >> 12));
    }
    constexpr inline MoveType type()
    {
        return static_cast<MoveType>(data & static_cast<uint16_t>(MoveType::MASK));
    }
    constexpr bool operator==(const Move &other) const
    {
        return data == other.data;
    }

    void set_promotion(PieceType pt){
        uint16_t mask = static_cast<uint16_t>(MoveType::PROMOTION_MASK) | static_cast<uint16_t>(MoveType::MASK);
        uint16_t code = static_cast<std::uint16_t>(MoveType::PROMOTION) | ((static_cast<uint16_t>(pt) - static_cast<uint16_t>(PieceType::KNIGHT)) << 12);
        data ^= (data & mask) ^ code;
    }
};

struct StateInfo
{
    uint16_t features;
    uint16_t rule50cnt;
    uint16_t enpassant_target_square;
    uint16_t captured_piece_code; // Тип и цвет взятой фигуры (или EMPTY)
    uint16_t captured_piece_sq;   // Используем Map::CNT_SQUARES если взятия не было

    // Конструктор по умолчанию для инициализации
    StateInfo(uint16_t features = 0,
              uint16_t rule50cnt = 0,
              uint16_t enpassant_target_square = static_cast<uint16_t>(Map::CNT_SQUARES),
              uint16_t captured_piece_code = static_cast<uint16_t>(PieceType::EMPTY),
              uint16_t captured_piece_sq = static_cast<uint16_t>(Map::CNT_SQUARES)) : features(features),
                                                                                      rule50cnt(0),
                                                                                      enpassant_target_square(enpassant_target_square),
                                                                                      captured_piece_code(captured_piece_code),
                                                                                      captured_piece_sq(captured_piece_sq)
    {
    }
};

struct Position
{

    std::array<uint16_t, static_cast<size_t>(Map::CNT_SQUARES)> board;
    std::array<uint16_t, 2> king_sq;
    std::array<Piece, static_cast<size_t>(Map::CNT_SQUARES) + 1> pieces_list; // +1 for the sake of simplisity out of range checking
    uint16_t end_pieces_list;

    uint16_t features;
    uint16_t rule50cnt;
    uint16_t enpassant_target_square;

    std::vector<StateInfo> state_history;
    std::vector<Move> moves;

    static constexpr bool get_side_to_move(const Position &pos)
    {
        return pos.features & static_cast<uint16_t>(Map::BIT_SIDE_TO_MOVE);
    }
    static constexpr std::string get_castling_rights_str(const Position &pos)
    {
        std::string s;
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_WK)))
            s += 'K';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_WQ)))
            s += 'Q';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_BK)))
            s += 'k';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_BQ)))
            s += 'q';
        return s.empty() ? "-" : s; // Возвращаем "-" если строка пуста
    }
    static constexpr std::string get_enpassant_str(const Position &pos)
    {
        return FEN::index_to_square(pos.enpassant_target_square);
    }

    Position(std::array<uint16_t, static_cast<uint16_t>(Map::CNT_SQUARES)> &board,
             uint16_t features = 0, uint16_t rule50cnt = 0, uint16_t enpassant_target_square = static_cast<uint16_t>(Map::CNT_SQUARES))
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

    Position(uint16_t features = 0, uint16_t rule50cnt = 0)
        : features(features), rule50cnt(rule50cnt), enpassant_target_square(static_cast<uint16_t>(Map::CNT_SQUARES)), end_pieces_list(0), moves(), state_history(), king_sq{static_cast<uint16_t>(Map::CNT_SQUARES), static_cast<uint16_t>(Map::CNT_SQUARES)}
    {
        pieces_list.fill(Piece::none());
        board.fill(static_cast<uint16_t>(Map::CNT_SQUARES));
    }

    void set_from_fen(std::string_view fen_view)
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

    void do_move(Move m)
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

    void undo_move()
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
};

inline std::string piece_type_to_string(PieceType pt) {
    switch (pt) {
        case PieceType::KING:   return "King";
        case PieceType::QUEEN:  return "Queen";
        case PieceType::ROOK:   return "Rook";
        case PieceType::BISHOP: return "Bishop";
        case PieceType::KNIGHT: return "Knight";
        case PieceType::PAWN:   return "Pawn";
        default:                return "Unknown";
    }
}

template <>
struct std::formatter<Piece> {
    // parse() остается пустым, так как мы не используем опции типа {:x} для Piece
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && *it != '}') {
            // В нашем случае здесь ничего нет, но это стандартная практика
            while (it != end && *it != '}') ++it;
        }
        return it;
    }

    // format() выполняет основную работу
    auto format(const Piece& p, std::format_context& ctx) const {
        auto out = ctx.out();
        
        if (p.type == static_cast<uint16_t>(PieceType::EMPTY)) {
            return std::format_to(out, "[Empty Piece]");
        }

        Color color = FEN::get_piece_color(p.type);
        PieceType type = FEN::get_piece_type(p.type);

        return std::format_to(out, "{} {} at {}",
                              (color == Color::WHITE ? "White" : "Black"),
                              piece_type_to_string(type),
                              FEN::index_to_square(p.position));
    }
};

template <>
struct std::formatter<Position> {
    // 'f' (full) - для вывода доски (по умолчанию)
    // 'l' (list) - для вывода списка фигур
    char presentation = 'f';

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'l')) {
            presentation = *it++;
        }
        
        // Убедимся, что после нашего флага идет '}'
        if (it != end && *it != '}') {
            throw std::format_error("invalid format specifier for Position");
        }
        
        return it;
    }

    // format() теперь использует 'presentation' для выбора формата вывода
    auto format(const Position& pos, std::format_context& ctx) const {
        auto out = ctx.out();
        
        if (presentation == 'l') {
            // Вывод списка фигур
            out = std::format_to(out, "Piece List (count: {}):\n", pos.end_pieces_list);
            for (uint16_t i = 0; i < pos.end_pieces_list; ++i) {
                // Используем форматер для Piece, который мы создали ранее
                out = std::format_to(out, "  [{:2}] {}\n", i, pos.pieces_list[i]);
            }
        } else { // presentation == 'f' (или по умолчанию)
            // Ваш существующий код для вывода доски
            out = std::format_to(out, "\n  +---+---+---+---+---+---+---+---+\n");
            for (int rank = static_cast<int>(Map::HEIGHT) - 1; rank >= 0; --rank) {
                out = std::format_to(out, "{} |", rank + 1);
                for (int file = 0; file < static_cast<int>(Map::WIDTH); ++file) {
                    int index = rank * static_cast<int>(Map::WIDTH) + file;
                    // Проверка, что на доске есть фигура
                    if (pos.board[index] < static_cast<uint16_t>(Map::CNT_SQUARES)) {
                        char piece_char = FEN::piece_to_fen_char(pos.pieces_list[pos.board[index]].type);
                        out = std::format_to(out, " {} |", piece_char);
                    } else {
                        out = std::format_to(out, "   |"); // Пустое поле
                    }
                }
                out = std::format_to(out, "\n  +---+---+---+---+---+---+---+---+\n");
            }
            out = std::format_to(out, "    a   b   c   d   e   f   g   h\n\n");
            out = std::format_to(out, "Cnt Pieces:   {}\n", pos.end_pieces_list);
            out = std::format_to(out, "Side to move: {}\n", (Position::get_side_to_move(pos) == static_cast<bool>(Color::WHITE) ? "White" : "Black"));
            out = std::format_to(out, "Castling:     {}\n", Position::get_castling_rights_str(pos));
            out = std::format_to(out, "En passant:   {}\n", Position::get_enpassant_str(pos));
            out = std::format_to(out, "Rule 50:      {}\n", pos.rule50cnt);
            out = std::format_to(out, "Features raw: {:#04x}\n", pos.features);
            out = std::format_to(out, "King squares : White {}, Black {}\n", FEN::index_to_square(pos.king_sq[static_cast<size_t>(Color::WHITE)]), FEN::index_to_square(pos.king_sq[static_cast<size_t>(Color::BLACK)]));
        }
        return out;
    }
};