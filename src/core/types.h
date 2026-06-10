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

// Move type flags (4 bits)
// Bits 12-15 encode move type:
// Bit 3 (value 8): Capture flag
// Bit 2 (value 4): Promotion flag
// Bits 0-1: Sub-type (00=Quiet/N, 01=DoublePush/B, 10=CastleKS/R, 11=CastleQS/Q)
enum MoveType : uint16_t {
    QUIET       = 0,      // 0000
    DOUBLE_PUSH = 1,      // 0001
    CASTLE_KS   = 2,      // 0010
    CASTLE_QS   = 3,      // 0011
    PROMO_N     = 4,      // 0100
    PROMO_B     = 5,      // 0101
    PROMO_R     = 6,      // 0110
    PROMO_Q     = 7,      // 0111
    CAPTURE     = 8,      // 1000
    EN_PASSANT  = 9,      // 1001
    PROMO_N_CAP = 12,     // 1100
    PROMO_B_CAP = 13,     // 1101
    PROMO_R_CAP = 14,     // 1110
    PROMO_Q_CAP = 15      // 1111
};

// New Move structure:
// Bits 0-5:  from square (6 bits)
// Bits 6-11: to square (6 bits)
// Bits 12-15: move type (4 bits)
struct Move {
    uint16_t data;
    
    Move() : data(0) {}
    Move(uint16_t d) : data(d) {}
    
    // Square accessors
    uint16_t from() const { return data & 0x3F; }
    uint16_t to() const { return (data >> 6) & 0x3F; }
    uint16_t type() const { return (data >> 12) & 0x0F; }
    
    // Setters
    void set_from(uint16_t f) { data = (data & 0xFFC0) | (f & 0x3F); }
    void set_to(uint16_t t) { data = (data & 0xFC3F) | ((t & 0x3F) << 6); }
    void set_type(uint16_t t) { data = (data & 0x0FFF) | ((t & 0x0F) << 12); }
    
    // Compatibility: piece() now returns move type for backward compatibility
    uint16_t piece() const { return type(); }
    void set_piece(uint16_t p) { set_type(p); }
    
    // Move type queries
    bool is_capture() const { return (type() & 8) != 0; }
    bool is_promotion() const { return (type() & 4) != 0; }
    bool is_quiet() const { return type() == QUIET; }
    bool is_double_push() const { return type() == DOUBLE_PUSH; }
    bool is_castle() const { return type() == CASTLE_KS || type() == CASTLE_QS; }
    bool is_kingside_castle() const { return type() == CASTLE_KS; }
    bool is_queenside_castle() const { return type() == CASTLE_QS; }
    bool is_en_passant() const { return type() == EN_PASSANT; }
    
    // Get promotion piece type (returns NONE if not a promotion)
    PieceType promotion_piece() const {
        if (!is_promotion()) return NONE;
        return static_cast<PieceType>(KNIGHT + (type() & 3));
    }
    
    // Set promotion type
    void set_promotion(PieceType pt, bool is_cap = false) {
        uint16_t t = 4 | (pt - KNIGHT);
        if (is_cap) t |= 8;
        set_type(t);
    }
    
    // Comparison operators
    bool operator==(const Move& other) const { return data == other.data; }
    bool operator!=(const Move& other) const { return data != other.data; }
    bool is_valid() const { return data != 0; }
};

// Note: Legacy SpecialMove enum removed - use MoveType instead

constexpr int BOARD_SIZE = 8;
constexpr int NUM_SQUARES = 64;
constexpr int NUM_PIECES = 6;
constexpr int NUM_COLORS = 2;

constexpr int MG_PIECE_VALUE[NUM_PIECES] = { 100, 320, 330, 500, 900, 20000 };

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
