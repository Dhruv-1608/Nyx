#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "types.h"

namespace Bitboards {
    // Precomputed attack tables
    extern Bitboard KnightAttacks[64];
    extern Bitboard KingAttacks[64];
    extern Bitboard PawnAttacks[2][64];

    void init();
}

#endif
