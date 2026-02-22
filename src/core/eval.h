#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include <cstdint>

class Board;

class Evaluator {
public:
    Evaluator();
    ~Evaluator() = default;
    int evaluate(const Board& board) const;
    int evaluate_material(const Board& board, Color c) const;
    int evaluate_position(const Board& board, Color c) const;
    int evaluate_mobility(const Board& board, Color c) const;
    int evaluate_king_safety(const Board& board, Color c) const;

private:
    static const int16_t MG_PST[NUM_PIECES][NUM_SQUARES];
    static const int16_t EG_PST[NUM_PIECES][NUM_SQUARES];
    static constexpr int MG_PIECE_VALUE[6] = { 100, 320, 330, 500, 900, 20000 };
    static constexpr int EG_PIECE_VALUE[6] = { 100, 320, 330, 500, 900, 20000 };
    static constexpr int PHASE_WEIGHTS[6] = { 0, 1, 1, 2, 4, 0 };
    int phase(const Board& board) const;
    int interpolate(int mg, int eg, int phase) const;
    Bitboard attacked_squares(const Board& board, Color c) const;
};

#endif
