#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "types.h"
#include <random>

namespace Zobrist {
    extern uint64_t PieceKeys[NUM_PIECES][NUM_COLORS][64];
    extern uint64_t EnPassantKeys[65]; // 64 possible squares + 1 for no en passant
    extern uint64_t CastleKeys[16];    // 16 possible castling rights states (4 bits)
    extern uint64_t SideToMoveKey;

    void init_keys();
}

#endif
