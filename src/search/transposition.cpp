#include "transposition.h"
#include "board.h"
#include "types.h"
#include <cstdlib>

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