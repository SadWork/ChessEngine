#pragma once
#include <bits/stdc++.h>

enum class Color : std::uint16_t
{
    BIT_COLOR = 1 << 3,
    LOG_BIT_COLOR = 3,
    WHITE = 0,
    BLACK = 1,
    UNSPECIFIED = 2,
};

enum class PieceType : std::uint16_t
{
    EMPTY,
    KING,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    MASK = 0x7,
};

enum class Map : std::uint16_t
{
    WIDTH = 8,
    HEIGHT = 8,
    CNT_SQUARES = WIDTH * HEIGHT,
    MAX_ATTACKS_PER_SQ = 16,
    MAX_MOVES = 218,
    BIT_SIDE_TO_MOVE = 1,
    BIT_NO_CASTLE_WK = 1 << 1,
    BIT_NO_CASTLE_WQ = 1 << 2,
    BIT_NO_CASTLE_BK = 1 << 3,
    BIT_NO_CASTLE_BQ = 1 << 4,
    LOG_BIT_SIDE_TO_MOVE = 0,
    LOG_BIT_NO_CASTLE_WK,
    LOG_BIT_NO_CASTLE_WQ,
    LOG_BIT_NO_CASTLE_BK,
    LOG_BIT_NO_CASTLE_BQ,
};

namespace FEN
{
    constexpr std::string_view Default("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
};

enum Direction : std::int16_t {
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

