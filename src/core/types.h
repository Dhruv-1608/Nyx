#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

enum PieceType : uint8_t {
    PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5, NONE = 6
};

enum Color : uint8_t {
    WHITE = 0, BLACK = 1
};

typedef uint8_t Square;
typedef uint64_t Bitboard;

struct Move {
    uint16_t data;
    Move() : data(0) {}
    Move(uint16_t d) : data(d) {}
    uint16_t from() const { return (data >> 0) & 0x3F; }
    uint16_t to() const { return (data >> 6) & 0x3F; }
    uint16_t piece() const { return (data >> 12) & 0x07; }
    uint16_t promotion() const { return (data >> 15) & 0x01; }
    void set_from(uint16_t f) { data = (data & 0xFFC0) | (f & 0x3F); }
    void set_to(uint16_t t) { data = (data & 0xFC3F) | ((t & 0x3F) << 6); }
    void set_piece(uint16_t p) { data = (data & 0xE1FF) | ((p & 0x07) << 12); }
    void set_promotion(uint16_t prom) { data = (data & 0x7FFF) | ((prom & 0x01) << 15); }
    bool is_promotion() const { return promotion() != 0; }
    bool is_capture() const { return false; }
};

enum SpecialMove : uint8_t {
    NORMAL = 0, CASTLE_KS = 1, CASTLE_QS = 2, EN_PASSANT = 3
};

constexpr int BOARD_SIZE = 8;
constexpr int NUM_SQUARES = 64;
constexpr int NUM_PIECES = 6;
constexpr int NUM_COLORS = 2;

constexpr Bitboard ALL_SQUARES = 0xFFFFFFFFFFFFFFFFULL;
constexpr Bitboard EMPTY = 0ULL;

extern Bitboard FILE_MASKS[8];
extern Bitboard RANK_MASKS[8];

constexpr Square SQ_A1 = 0, SQ_B1 = 1, SQ_C1 = 2, SQ_D1 = 3, SQ_E1 = 4, SQ_F1 = 5, SQ_G1 = 6, SQ_H1 = 7;
constexpr Square SQ_A8 = 56, SQ_B8 = 57, SQ_C8 = 58, SQ_D8 = 59, SQ_E8 = 60, SQ_F8 = 61, SQ_G8 = 62, SQ_H8 = 63;

inline Square make_square(int file, int rank) { return (rank * 8) + file; }
inline int file_of(Square s) { return s % 8; }
inline int rank_of(Square s) { return s / 8; }
inline Bitboard set_bit(Bitboard bb, Square s) { return bb | (1ULL << s); }
inline Bitboard get_bit(Bitboard bb, Square s) { return (bb >> s) & 1ULL; }
inline Bitboard pop_lsb(Bitboard& bb) {
    Bitboard lsb = bb & -bb;
    bb ^= lsb;
    return lsb;
}

#endif
