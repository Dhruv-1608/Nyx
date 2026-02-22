#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "types.h"
#include <vector>

class Board;

struct MoveList {
    std::vector<Move> moves;
    void clear() { moves.clear(); }
    void add_move(Move m) { moves.push_back(m); }
    size_t size() const { return moves.size(); }
    const Move& operator[](size_t i) const { return moves[i]; }
};

class MoveGenerator {
public:
    MoveGenerator(const Board& board);
    ~MoveGenerator() = default;
    MoveList generate_all() const;
    MoveList generate_pseudo_legal() const;
    bool is_pseudo_legal(const Move& move) const;
    Bitboard attacks_to_square(Square sq, Color attacker) const;

private:
    const Board& m_board;
    void gen_pawn_moves(Square sq, Color c, MoveList& list) const;
    void gen_knight_moves(Square sq, Color c, MoveList& list) const;
    void gen_bishop_moves(Square sq, Color c, MoveList& list) const;
    void gen_rook_moves(Square sq, Color c, MoveList& list) const;
    void gen_queen_moves(Square sq, Color c, MoveList& list) const;
    void gen_king_moves(Square sq, Color c, MoveList& list) const;
    Bitboard pawn_attacks(Square sq, Color c) const;
    bool in_check(Color c) const;
    Square king_square(Color c) const;
    Bitboard bishop_moves_bb(Square sq, Bitboard occupied) const;
    Bitboard rook_moves_bb(Square sq, Bitboard occupied) const;
    Bitboard queen_moves_bb(Square sq, Bitboard occupied) const;
};

#endif
