#ifndef MOVE_VALIDATOR_H
#define MOVE_VALIDATOR_H

#include "types.h"
#include "board.h"
#include "movegen.h"

class MoveValidator {
public:
    // Check if a move is legal in the current position
    static bool is_legal(const Board& board, const Move& move);
    
    // Get all legal moves for the current position
    static MoveList get_legal_moves(const Board& board);
    
    // Validate and fix a move if needed - returns a legal move
    static Move validate_and_fix(const Board& board, const Move& suggested_move);
};

#endif // MOVE_VALIDATOR_H
