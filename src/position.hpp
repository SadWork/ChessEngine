#pragma once

#include "types.h"
#include <bits/stdc++.h>


namespace FEN
{
    constexpr inline PieceType get_piece_type(uint16_t piece_code)
    {
        return static_cast<PieceType>(static_cast<uint16_t>(piece_code) & static_cast<uint16_t>(PieceType::MASK)); // Младшие 3 бита - тип
    }

    constexpr inline Color get_piece_color(uint16_t piece_code)
    {
        return static_cast<Color>((static_cast<uint16_t>(piece_code) & static_cast<uint16_t>(Color::BIT_COLOR)) >> static_cast<uint16_t>(Color::LOG_BIT_COLOR));
    }

    constexpr inline uint16_t make_piece_code(Color color, PieceType type)
    {
        return (static_cast<uint16_t>(color) << static_cast<uint16_t>(Color::LOG_BIT_COLOR)) | static_cast<uint16_t>(type);
    }

    constexpr inline uint16_t fen_char_to_piece(char c)
    {
        switch (c)
        {
        case 'P': return make_piece_code(Color::WHITE, PieceType::PAWN);
        case 'N': return make_piece_code(Color::WHITE, PieceType::KNIGHT);
        case 'B': return make_piece_code(Color::WHITE, PieceType::BISHOP);
        case 'R': return make_piece_code(Color::WHITE, PieceType::ROOK);
        case 'Q': return make_piece_code(Color::WHITE, PieceType::QUEEN);
        case 'K': return make_piece_code(Color::WHITE, PieceType::KING);
        case 'p': return make_piece_code(Color::BLACK, PieceType::PAWN);
        case 'n': return make_piece_code(Color::BLACK, PieceType::KNIGHT);
        case 'b': return make_piece_code(Color::BLACK, PieceType::BISHOP);
        case 'r': return make_piece_code(Color::BLACK, PieceType::ROOK);
        case 'q': return make_piece_code(Color::BLACK, PieceType::QUEEN);
        case 'k': return make_piece_code(Color::BLACK, PieceType::KING);
        default:  return static_cast<uint16_t>(PieceType::EMPTY);
        }
    }

    constexpr inline int square_to_index(std::string_view sq_str)
    {
        if (sq_str.length() != 2) return -1;
        char file_char = sq_str[0];
        char rank_char = sq_str[1];
        if (file_char < 'a' || file_char > 'h' || rank_char < '1' || rank_char > '8') return -1;

        int file = file_char - 'a';
        int rank = rank_char - '1';
        return rank * static_cast<int>(Map::WIDTH) + file;
    }
    
    constexpr inline uint16_t dist(uint16_t from, uint16_t to)
    {
        uint16_t x1{static_cast<uint16_t>(from & 0x7)};
        uint16_t y1{static_cast<uint16_t>(from >> 3)};
        uint16_t x2{static_cast<uint16_t>(to & 0x7)};
        uint16_t y2{static_cast<uint16_t>(to >> 3)};
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
        case PieceType::EMPTY:  c = '.'; break;
        case PieceType::PAWN:   c = 'P'; break;
        case PieceType::KNIGHT: c = 'N'; break;
        case PieceType::BISHOP: c = 'B'; break;
        case PieceType::ROOK:   c = 'R'; break;
        case PieceType::QUEEN:  c = 'Q'; break;
        case PieceType::KING:   c = 'K'; break;
        }
        return color == Color::BLACK ? std::tolower(c) : c;
    }

    // Преобразует индекс (0-63) в строку поля ("a1"-"h8")
    inline std::string index_to_square(int index)
    {
        if (index < 0 || index >= static_cast<int>(Map::CNT_SQUARES)) return "??";
        char file = 'a' + (index % static_cast<int>(Map::WIDTH));
        char rank = '1' + (index / static_cast<int>(Map::WIDTH));
        return {file, rank};
    }
} // namespace FEN


struct Piece
{
    uint16_t type, position;

    static consteval Piece none()
    {
        return {static_cast<uint16_t>(PieceType::EMPTY), static_cast<uint16_t>(Map::CNT_SQUARES)};
    }
};


enum class MoveType : std::uint16_t
{
    NORMAL        = 0 << 14,
    PROMOTION     = 1 << 14,
    EN_PASSANT    = 2 << 14,
    CASTLING      = 3 << 14,
    MASK          = 3 << 14,
    PROMOTION_MASK= 3 << 12,
};


struct Move
{
    std::uint16_t data;

    constexpr explicit Move(std::uint16_t d = 0) : data(d) {}
    constexpr Move(uint16_t from, uint16_t to, MoveType mt = MoveType::NORMAL)
        : data(static_cast<std::uint16_t>(mt) | (std::uint16_t(from) << 6) | to) {}
    constexpr Move(uint16_t from, uint16_t to, PieceType pt)
        : data(static_cast<std::uint16_t>(MoveType::PROMOTION) | ((static_cast<uint16_t>(pt) - static_cast<uint16_t>(PieceType::KNIGHT)) << 12) | (from << 6) | to) {}

    static consteval Move null() { return Move(static_cast<uint16_t>(Map::CNT_SQUARES) | (static_cast<uint16_t>(Map::CNT_SQUARES) << 6)); }
    static consteval Move none() { return Move(0); }

    constexpr inline uint16_t dest() const { return data & 0x3F; }
    constexpr inline uint16_t source() const { return (data >> 6) & 0x3F; }
    constexpr inline PieceType promotion_piece() const { return PieceType(static_cast<uint16_t>(PieceType::KNIGHT) + ((data & static_cast<uint16_t>(MoveType::PROMOTION_MASK)) >> 12)); }
    constexpr inline MoveType type() const { return static_cast<MoveType>(data & static_cast<uint16_t>(MoveType::MASK)); }
    constexpr bool operator==(const Move& other) const { return data == other.data; }

    inline void set_promotion(PieceType pt) {
        uint16_t mask = static_cast<uint16_t>(MoveType::PROMOTION_MASK) | static_cast<uint16_t>(MoveType::MASK);
        uint16_t code = static_cast<std::uint16_t>(MoveType::PROMOTION) | ((static_cast<uint16_t>(pt) - static_cast<uint16_t>(PieceType::KNIGHT)) << 12);
        data = (data & ~mask) | code;
    }
};


struct StateInfo
{
    uint16_t features;
    uint16_t rule50cnt;
    uint16_t enpassant_target_square;
    uint16_t captured_piece_code;
    uint16_t captured_piece_sq;

    StateInfo(uint16_t features = 0,
              uint16_t rule50cnt = 0,
              uint16_t enpassant_target_square = static_cast<uint16_t>(Map::CNT_SQUARES),
              uint16_t captured_piece_code = static_cast<uint16_t>(PieceType::EMPTY),
              uint16_t captured_piece_sq = static_cast<uint16_t>(Map::CNT_SQUARES)) 
        : features(features),
          rule50cnt(rule50cnt),
          enpassant_target_square(enpassant_target_square),
          captured_piece_code(captured_piece_code),
          captured_piece_sq(captured_piece_sq)
    {}
};


struct Position
{
    // === ДАННЫЕ ===
    std::array<uint16_t, static_cast<size_t>(Map::CNT_SQUARES)> board;
    std::array<uint16_t, 2> king_sq;
    std::array<Piece, static_cast<size_t>(Map::CNT_SQUARES) + 1> pieces_list;
    uint16_t end_pieces_list;

    uint16_t features;
    uint16_t rule50cnt;
    uint16_t enpassant_target_square;

    std::vector<StateInfo> state_history;
    std::vector<Move> moves;
    
    // === СТАТИЧЕСКИЕ ХЕЛПЕРЫ ===
    static constexpr bool get_side_to_move(const Position& pos) {
        return pos.features & static_cast<uint16_t>(Map::BIT_SIDE_TO_MOVE);
    }
    static constexpr std::string get_castling_rights_str(const Position& pos) {
        std::string s;
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_WK))) s += 'K';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_WQ))) s += 'Q';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_BK))) s += 'k';
        if (!(pos.features & static_cast<uint16_t>(Map::BIT_NO_CASTLE_BQ))) s += 'q';
        return s.empty() ? "-" : s;
    }
    static constexpr std::string get_enpassant_str(const Position& pos) {
        return FEN::index_to_square(pos.enpassant_target_square);
    }
    
    // === ОБЪЯВЛЕНИЯ МЕТОДОВ ===
    Position(uint16_t features = 0, uint16_t rule50cnt = 0);
    Position(std::array<uint16_t, static_cast<uint16_t>(Map::CNT_SQUARES)>& board,
             uint16_t features = 0, uint16_t rule50cnt = 0, 
             uint16_t enpassant_target_square = static_cast<uint16_t>(Map::CNT_SQUARES));
    
    void set_from_fen(std::string_view fen_view);
    void do_move(Move m);
    void undo_move();
};

// === ШАБЛОННЫЕ СПЕЦИАЛИЗАЦИИ ===

// Вспомогательная inline-функция для форматера
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
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const Piece& p, std::format_context& ctx) const {
        if (p.type == static_cast<uint16_t>(PieceType::EMPTY)) {
            return std::format_to(ctx.out(), "[Empty Piece]");
        }
        Color color = FEN::get_piece_color(p.type);
        PieceType type = FEN::get_piece_type(p.type);
        return std::format_to(ctx.out(), "{} {} at {}",
                              (color == Color::WHITE ? "White" : "Black"),
                              piece_type_to_string(type),
                              FEN::index_to_square(p.position));
    }
};

template <>
struct std::formatter<Position> {
    char presentation = 'f';

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'f' || *it == 'l')) {
            presentation = *it++;
        }
        if (it != end && *it != '}') {
            throw std::format_error("invalid format specifier for Position");
        }
        return it;
    }

    auto format(const Position& pos, std::format_context& ctx) const {
        auto out = ctx.out();
        if (presentation == 'l') {
            out = std::format_to(out, "Piece List (count: {}):\n", pos.end_pieces_list);
            for (uint16_t i = 0; i < pos.end_pieces_list; ++i) {
                out = std::format_to(out, "  [{:2}] {}\n", i, pos.pieces_list[i]);
            }
        } else {
            out = std::format_to(out, "\n  +---+---+---+---+---+---+---+---+\n");
            for (int rank = static_cast<int>(Map::HEIGHT) - 1; rank >= 0; --rank) {
                out = std::format_to(out, "{} |", rank + 1);
                for (int file = 0; file < static_cast<int>(Map::WIDTH); ++file) {
                    int index = rank * static_cast<int>(Map::WIDTH) + file;
                    if (pos.board[index] < static_cast<uint16_t>(Map::CNT_SQUARES)) {
                        char piece_char = FEN::piece_to_fen_char(pos.pieces_list[pos.board[index]].type);
                        out = std::format_to(out, " {} |", piece_char);
                    } else {
                        out = std::format_to(out, "   |");
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

// Форматер для вывода Move в UCI-нотации, например, "e2e4" или "a7a8q"
template <>
struct std::formatter<Move> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const Move& m, std::format_context& ctx) const {
        if (m == Move::none()) {
            return std::format_to(ctx.out(), "none");
        }
        if (m == Move::null()) {
            return std::format_to(ctx.out(), "null");
        }

        std::string s = FEN::index_to_square(m.source()) + FEN::index_to_square(m.dest());

        if (m.type() == MoveType::PROMOTION) {
            switch (m.promotion_piece()) {
                case PieceType::QUEEN:  s += 'q'; break;
                case PieceType::ROOK:   s += 'r'; break;
                case PieceType::BISHOP: s += 'b'; break;
                case PieceType::KNIGHT: s += 'n'; break;
                default:                s += '?'; break;
            }
        }
        return std::format_to(ctx.out(), "{}", s);
    }
};