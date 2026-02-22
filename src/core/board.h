#ifndef BOARD_H
#define BOARD_H

#include "types.h"
#include <string>
#include <array>

class Board {
public:
    Board();
    ~Board() = default;
    void reset();
    bool load_fen(const std::string& fen);
    Bitboard pieces(PieceType pt, Color c) const { return m_pieces[pt][c]; }
    Bitboard all_pieces(Color c) const { return m_colors[c]; }
    Bitboard all_pieces() const { return m_colors[0] | m_colors[1]; }
    Square en_passant() const { return m_en_passant; }
    bool can_castle(Color c, bool kingside) const;
    uint8_t castle_rights() const { return m_castle_rights; }
    Color side_to_move() const { return m_side; }
    int halfmove_clock() const { return m_halfmove; }
    int fullmove_number() const { return m_fullmove; }
    void make_move(const Move& move);
    void unmake_move(const Move& move);
    std::string to_fen() const;
    Square find_king(Color c) const;

private:
    Bitboard m_pieces[NUM_PIECES][NUM_COLORS];
    Bitboard m_colors[NUM_COLORS];
    Color m_side;
    Square m_en_passant;
    uint8_t m_castle_rights;
    int m_halfmove;
    int m_fullmove;
    struct State {
        Bitboard pieces[NUM_PIECES][NUM_COLORS];
        Bitboard colors[NUM_COLORS];
        Color side;
        Square en_passant;
        uint8_t castle_rights;
        int halfmove;
    };
    std::array<State, 256> m_history;
    int m_history_ply;
    void update_castling_rights(Square from, Square to);
    void clear_piece(Square sq, PieceType pt, Color c);
    void place_piece(Square sq, PieceType pt, Color c);
};

#endif
