#include "move_validator.h"

bool MoveValidator::is_legal(const Board& board, const Move& move) {
    MoveGenerator mg(board);
    MoveList legal_moves = mg.generate_all();
    
    // Match on from/to squares and promotion status only.
    // The move type (quiet, capture, castling) is determined by the board state
    // and we don't want to reject a valid move due to a type mismatch in the caller.
    for (const Move& m : legal_moves) {
        if (m.from() == move.from() && m.to() == move.to()) {
            // For promotions, also match the promotion piece type
            if (m.is_promotion() != move.is_promotion()) continue;
            if (m.is_promotion() && m.promotion_piece() != move.promotion_piece()) continue;
            return true;
        }
    }
    return false;
}

MoveList MoveValidator::get_legal_moves(const Board& board) {
    MoveGenerator mg(board);
    return mg.generate_all();
}

Move MoveValidator::validate_and_fix(const Board& board, const Move& suggested_move) {
    MoveGenerator mg(board);
    MoveList legal_moves = mg.generate_all();
    
    if (legal_moves.size() == 0) {
        return Move(); // No legal moves - return invalid move
    }
    
    // First check if the suggested move is legal (matching from, to, AND type)
    for (const Move& m : legal_moves) {
        if (m.from() == suggested_move.from() && m.to() == suggested_move.to() && m.type() == suggested_move.type()) {
            return suggested_move; // Move is legal
        }
    }
    
    // If from/to match but type doesn't, return the correct move with proper type
    for (const Move& m : legal_moves) {
        if (m.from() == suggested_move.from() && m.to() == suggested_move.to()) {
            return m; // Return the legal move with correct type
        }
    }
    
    // Move is completely illegal - return first legal move
    return legal_moves[0];
}
