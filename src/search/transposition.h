#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "types.h"
#include <cstdint>

class Board;

struct TTEntry {
    uint64_t key;
    int16_t score;
    uint8_t depth;
    uint8_t flag;
    Move best_move;
    TTEntry() : key(0), score(0), depth(0), flag(0), best_move() {}
};

enum TTFlag : uint8_t {
    EXACT = 0,
    LOWERBOUND = 1,
    UPPERBOUND = 2
};

class TranspositionTable {
public:
    TranspositionTable(size_t size_mb = 16);
    ~TranspositionTable();
    void init(size_t size_mb);
    void clear();
    void store(uint64_t key, int score, uint8_t depth, TTFlag flag, const Move& move);
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& move);

private:
    TTEntry* m_table;
    size_t m_size;
    size_t m_mask;
};

uint64_t compute_zobrist(const Board& board);

#endif
