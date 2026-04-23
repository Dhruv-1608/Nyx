#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

constexpr int MG_PIECE_VALUE[NUM_PIECES] = { 100, 320, 330, 500, 900, 20000 };


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
// Bit 15: capture flag
// Bits 12-14: special move type (0=quiet, 1=double push, 2=castle KS, 3=castle QS, 4=en passant, 5-8=promotions)
enum MoveType : uint16_t {
    QUIET       = 0,      // 0000 - quiet move
    DOUBLE_PUSH = 1,      // 0001 - pawn double push
    CASTLE_KS   = 2,      // 0010 - kingside castle
    CASTLE_QS   = 3,      // 0011 - queenside castle
    EN_PASSANT  = 4,      // 0100 - en passant capture
    PROMO_N     = 5,      // 0101 - knight promotion
    PROMO_B     = 6,      // 0110 - bishop promotion
    PROMO_R     = 7,      // 0111 - rook promotion
    PROMO_Q     = 8,      // 1000 - queen promotion
    CAPTURE     = 0x8,    // 1000 bit - capture flag (can be ORed with move types 0-8)
    // Combined capture+promotion types
    PROMO_N_CAP = CAPTURE | PROMO_N,  // 1101
    PROMO_B_CAP = CAPTURE | PROMO_B,  // 1110
    PROMO_R_CAP = CAPTURE | PROMO_R,  // 1111
    PROMO_Q_CAP = CAPTURE | PROMO_Q,  // 1000 (same as CAPTURE, use PROMOTION flag to distinguish)
};

// New Move structure:
// Bits 0-5:  from square (6 bits)
// Bits 6-11: to square (6 bits)
// Bits 12-15: move type (4 bits) - encodes special moves, captures, and promotions
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
    // Deprecated - use type() instead
    uint16_t piece() const { return type(); }
    void set_piece(uint16_t p) { set_type(p); }
    
    // Move type queries
    bool is_capture() const { return (data & (CAPTURE << 12)) != 0 || type() == EN_PASSANT; }
    bool is_promotion() const { 
        uint16_t t = type();
        return t >= PROMO_N && t <= PROMO_Q;  // Promotions 5-8
    }
    bool is_quiet() const { return type() == QUIET; }
    bool is_double_push() const { return type() == DOUBLE_PUSH; }
    bool is_castle() const { 
        uint16_t t = type();
        return t == CASTLE_KS || t == CASTLE_QS; 
    }
    bool is_kingside_castle() const { return type() == CASTLE_KS; }
    bool is_queenside_castle() const { return type() == CASTLE_QS; }
    bool is_en_passant() const { return type() == EN_PASSANT; }
    
    // Get promotion piece type (returns NONE if not a promotion)
    PieceType promotion_piece() const {
        uint16_t t = type();
        if (t == PROMO_N || t == PROMO_N_CAP) return KNIGHT;
        if (t == PROMO_B || t == PROMO_B_CAP) return BISHOP;
        if (t == PROMO_R || t == PROMO_R_CAP) return ROOK;
        if (t == PROMO_Q || t == PROMO_Q_CAP) return QUEEN;
        return NONE;
    }
    
    // Set promotion type
    void set_promotion(PieceType pt, bool is_cap = false) {
        uint16_t base = 0;
        switch (pt) {
            case KNIGHT: base = PROMO_N; break;
            case BISHOP: base = PROMO_B; break;
            case ROOK:   base = PROMO_R; break;
            case QUEEN:  base = PROMO_Q; break;
            default: break;
        }
        if (is_cap) base |= CAPTURE;
        set_type(base);
    }
    
    // Compatibility: promotion() returns 1 if promotion, 0 otherwise
    uint16_t promotion() const { return is_promotion() ? 1 : 0; }
    void set_promotion(uint16_t prom) { 
        // Deprecated - kept for compatibility
        if (prom) set_type(PROMO_Q); 
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
