#include "transposition.h"
#include "board.h"
#include "types.h"
#include <cstdlib>

namespace {
    uint64_t zobrist_keys[6][2][64];
    uint64_t zobrist_side;
    uint64_t zobrist_castle[16];
    uint64_t zobrist_ep[64];
    bool zobrist_initialized = false;

    uint64_t random64() {
        static uint64_t seed = 123456789ULL;
        seed ^= seed << 13;
        seed ^= seed >> 7;
        seed ^= seed << 17;
        return seed;
    }

    void init_zobrist() {
        if (zobrist_initialized) return;
        for (int pt = 0; pt < 6; ++pt) {
            for (int c = 0; c < 2; ++c) {
                for (int sq = 0; sq < 64; ++sq) {
                    zobrist_keys[pt][c][sq] = random64();
                }
            }
        }
        zobrist_side = random64();
        for (int i = 0; i < 16; ++i) {
            zobrist_castle[i] = random64();
        }
        for (int sq = 0; sq < 64; ++sq) {
            zobrist_ep[sq] = random64();
        }
        zobrist_initialized = true;
    }
}

TranspositionTable::TranspositionTable(size_t size_mb) : m_table(nullptr), m_size(0), m_mask(0) {
    init(size_mb);
}

TranspositionTable::~TranspositionTable() {
    clear();
}

void TranspositionTable::init(size_t size_mb) {
    clear();
    size_t entries = (size_mb * 1024 * 1024) / 32;
    size_t power = 1;
    while (power < entries) power <<= 1;
    m_size = power;
    m_mask = m_size - 1;
    m_table = new TTEntry[m_size];
}

void TranspositionTable::clear() {
    if (m_table) {
        delete[] m_table;
        m_table = nullptr;
    }
    m_size = 0;
    m_mask = 0;
}

void TranspositionTable::store(uint64_t key, int score, uint8_t depth, TTFlag flag, const Move& move) {
    if (!m_table) return;
    size_t idx = key & m_mask;
    TTEntry& entry = m_table[idx];
    if (entry.key == 0 || depth >= entry.depth) {
        entry.key = key;
        entry.score = static_cast<int16_t>(score);
        entry.depth = depth;
        entry.flag = static_cast<uint8_t>(flag);
        entry.best_move = move;
    }
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& move) {
    if (!m_table) return false;

    size_t idx = key & m_mask;
    const TTEntry& entry = m_table[idx];

    if (entry.key != key || entry.depth < static_cast<uint8_t>(depth)) {
        return false;
    }

    score = entry.score;
    move = entry.best_move;

    // Check if we can use this entry for cutoffs
    if (entry.flag == EXACT) {
        return true;
    } else if (entry.flag == LOWERBOUND && score >= beta) {
        return true;
    } else if (entry.flag == UPPERBOUND && score <= alpha) {
        return true;
    }

    return false;
}

// Compute Zobrist hash for a board position
uint64_t compute_zobrist(const Board& board) {
    if (!zobrist_initialized) {
        init_zobrist();
    }

    uint64_t hash = 0;

    // Pieces
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        for (int color_idx = 0; color_idx < NUM_COLORS; ++color_idx) {
            Color c = static_cast<Color>(color_idx);
            Bitboard bb = board.pieces(static_cast<PieceType>(pt), c);
            while (bb) {
                Square sq = static_cast<Square>(__builtin_ctzll(bb));
                hash ^= zobrist_keys[pt][color_idx][sq];
                bb &= bb - 1;
            }
        }
    }

    // Side to move
    if (board.side_to_move() == BLACK) {
        hash ^= zobrist_side;
    }

    // Castling rights
    uint8_t castle = board.castle_rights();
    hash ^= zobrist_castle[castle];

    // En passant
    Square ep = board.en_passant();
    if (ep != 64) {
        hash ^= zobrist_ep[ep];
    }

    return hash;
}
