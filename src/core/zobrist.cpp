#include "zobrist.h"
#include <random>

namespace Zobrist {
    uint64_t PieceKeys[NUM_PIECES][NUM_COLORS][64];
    uint64_t EnPassantKeys[65];
    uint64_t CastleKeys[16];
    uint64_t SideToMoveKey;

    void init_keys() {
        std::mt19937_64 rng(12345ULL); // Fixed seed for reproducibility

        for (int pt = 0; pt < NUM_PIECES; ++pt) {
            for (int c = 0; c < NUM_COLORS; ++c) {
                for (int sq = 0; sq < 64; ++sq) {
                    PieceKeys[pt][c][sq] = rng();
                }
            }
        }

        for (int i = 0; i < 65; ++i) {
            EnPassantKeys[i] = rng();
        }

        for (int i = 0; i < 16; ++i) {
            CastleKeys[i] = rng();
        }

        SideToMoveKey = rng();
    }
}
