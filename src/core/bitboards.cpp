#include "bitboards.h"

namespace Bitboards {
    Bitboard KnightAttacks[64];
    Bitboard KingAttacks[64];
    Bitboard PawnAttacks[2][64];

    void init() {
        for (int sq = 0; sq < 64; ++sq) {
            int file = sq % 8;
            int rank = sq / 8;

            // Knight
            int kn_dx[] = { -2, -1, 1, 2, 2, 1, -1, -2 };
            int kn_dy[] = { 1, 2, 2, 1, -1, -2, -2, -1 };
            for (int i = 0; i < 8; ++i) {
                int nx = file + kn_dx[i];
                int ny = rank + kn_dy[i];
                if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                    KnightAttacks[sq] |= (1ULL << (ny * 8 + nx));
                }
            }

            // King
            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = file + dx;
                    int ny = rank + dy;
                    if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                        KingAttacks[sq] |= (1ULL << (ny * 8 + nx));
                    }
                }
            }

            // Pawn
            if (rank > 0 && rank < 7) {
                if (file > 0) PawnAttacks[WHITE][sq] |= (1ULL << ((rank + 1) * 8 + (file - 1)));
                if (file < 7) PawnAttacks[WHITE][sq] |= (1ULL << ((rank + 1) * 8 + (file + 1)));
                if (file > 0) PawnAttacks[BLACK][sq] |= (1ULL << ((rank - 1) * 8 + (file - 1)));
                if (file < 7) PawnAttacks[BLACK][sq] |= (1ULL << ((rank - 1) * 8 + (file + 1)));
            }
        }
    }
}
