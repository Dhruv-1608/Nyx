#include "board.h"
#include <sstream>
#include <algorithm>

// File and rank masks
Bitboard FILE_MASKS[8];
Bitboard RANK_MASKS[8];

namespace {
    void init_masks() {
        for (int f = 0; f < 8; ++f) {
            FILE_MASKS[f] = 0;
            for (int r = 0; r < 8; ++r) {
                FILE_MASKS[f] |= 1ULL << make_square(f, r);
            }
        }
        for (int r = 0; r < 8; ++r) {
            RANK_MASKS[r] = 0;
            for (int f = 0; f < 8; ++f) {
                RANK_MASKS[r] |= 1ULL << make_square(f, r);
            }
        }
    }
}

Board::Board() {
    init_masks();
    reset();
}

void Board::reset() {
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        for (int c = 0; c < NUM_COLORS; ++c) {
            m_pieces[pt][c] = 0;
        }
    }
    for (int c = 0; c < NUM_COLORS; ++c) {
        m_colors[c] = 0;
    }

    // Starting position
    m_pieces[PAWN][WHITE]   = 0x000000000000FF00ULL;
    m_pieces[KNIGHT][WHITE] = 0x0000000000000042ULL;
    m_pieces[BISHOP][WHITE] = 0x0000000000000024ULL;
    m_pieces[ROOK][WHITE]   = 0x0000000000000081ULL;
    m_pieces[QUEEN][WHITE]  = 0x0000000000000008ULL;
    m_pieces[KING][WHITE]   = 0x0000000000000010ULL;

    m_pieces[PAWN][BLACK]   = 0x00FF000000000000ULL;
    m_pieces[KNIGHT][BLACK] = 0x4200000000000000ULL;
    m_pieces[BISHOP][BLACK] = 0x2400000000000000ULL;
    m_pieces[ROOK][BLACK]   = 0x8100000000000000ULL;
    m_pieces[QUEEN][BLACK]  = 0x0800000000000000ULL;
    m_pieces[KING][BLACK]   = 0x1000000000000000ULL;

    // Update colors
    for (int c = 0; c < NUM_COLORS; ++c) {
        m_colors[c] = 0;
        for (int pt = 0; pt < NUM_PIECES; ++pt) {
            m_colors[c] |= m_pieces[pt][c];
        }
    }

    m_side = WHITE;
    m_en_passant = 64;
    m_castle_rights = 0b1111;
    m_halfmove = 0;
    m_fullmove = 1;
    m_history_ply = 0;
}

bool Board::load_fen(const std::string& fen) {
    std::istringstream ss(fen);
    std::string token;

    if (!(ss >> token)) return false;

    // Clear
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        for (int c = 0; c < NUM_COLORS; ++c) {
            m_pieces[pt][c] = 0;
        }
    }

    int rank = 7;
    int file = 0;

    for (char c : token) {
        if (c == '/') {
            if (file != 8) return false;
            rank--;
            file = 0;
            if (rank < 0) return false;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
            if (file > 8) return false;
        } else {
            PieceType pt = NONE;
            Color col = WHITE;
            switch (c) {
                case 'P': pt = PAWN; col = WHITE; break;
                case 'N': pt = KNIGHT; col = WHITE; break;
                case 'B': pt = BISHOP; col = WHITE; break;
                case 'R': pt = ROOK; col = WHITE; break;
                case 'Q': pt = QUEEN; col = WHITE; break;
                case 'K': pt = KING; col = WHITE; break;
                case 'p': pt = PAWN; col = BLACK; break;
                case 'n': pt = KNIGHT; col = BLACK; break;
                case 'b': pt = BISHOP; col = BLACK; break;
                case 'r': pt = ROOK; col = BLACK; break;
                case 'q': pt = QUEEN; col = BLACK; break;
                case 'k': pt = KING; col = BLACK; break;
                default: return false;
            }

            if (file >= 8) return false;
            Square sq = make_square(file, rank);
            m_pieces[pt][col] |= 1ULL << sq;
            file++;
        }
    }

    if (rank != 0 || file != 8) return false;

    // Colors
    for (int c = 0; c < NUM_COLORS; ++c) {
        m_colors[c] = 0;
        for (int pt = 0; pt < NUM_PIECES; ++pt) {
            m_colors[c] |= m_pieces[pt][c];
        }
    }

    // Color
    if (!(ss >> token)) return false;
    m_side = (token == "w") ? WHITE : BLACK;

    // Castling
    if (!(ss >> token)) return false;
    m_castle_rights = 0;
    for (char c : token) {
        switch (c) {
            case 'K': m_castle_rights |= 0b0001; break;
            case 'Q': m_castle_rights |= 0b0010; break;
            case 'k': m_castle_rights |= 0b0100; break;
            case 'q': m_castle_rights |= 0b1000; break;
        }
    }

    // EP
    if (ss >> token) {
        if (token != "-") {
            if (token.length() != 2) return false;
            int f = token[0] - 'a';
            int r = token[1] - '1';
            if (f < 0 || f > 7 || r < 0 || r > 7) return false;
            m_en_passant = make_square(f, r);
        } else {
            m_en_passant = 64;
        }
    } else {
        m_en_passant = 64;
    }

    // Halfmove
    if (ss >> m_halfmove) {
        // Fullmove
        if (ss >> m_fullmove) {
            // ok
        } else {
            m_fullmove = 1;
        }
    } else {
        m_halfmove = 0;
        m_fullmove = 1;
    }

    m_history_ply = 0;
    return true;
}

bool Board::can_castle(Color c, bool kingside) const {
    if (kingside) {
        return (c == WHITE && (m_castle_rights & 0b0001)) ||
               (c == BLACK && (m_castle_rights & 0b0100));
    } else {
        return (c == WHITE && (m_castle_rights & 0b0010)) ||
               (c == BLACK && (m_castle_rights & 0b1000));
    }
}

Square Board::find_king(Color c) const {
    Bitboard king_bb = m_pieces[KING][c];
    if (king_bb == 0) return 64;
    return static_cast<Square>(__builtin_ctzll(king_bb));
}

void Board::clear_piece(Square sq, PieceType pt, Color c) {
    m_pieces[pt][c] &= ~(1ULL << sq);
    m_colors[c] &= ~(1ULL << sq);
}

void Board::place_piece(Square sq, PieceType pt, Color c) {
    m_pieces[pt][c] |= 1ULL << sq;
    m_colors[c] |= 1ULL << sq;
}

// Helper function to find piece type at a square
PieceType piece_at_square(const Board& board, Square sq, Color c) {
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        if (board.pieces(static_cast<PieceType>(pt), c) & (1ULL << sq)) {
            return static_cast<PieceType>(pt);
        }
    }
    return NONE;
}

void Board::make_move(const Move& move) {
    // Save state
    State& st = m_history[m_history_ply];
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        for (int c = 0; c < NUM_COLORS; ++c) {
            st.pieces[pt][c] = m_pieces[pt][c];
        }
    }
    for (int c = 0; c < NUM_COLORS; ++c) {
        st.colors[c] = m_colors[c];
    }
    st.side = m_side;
    st.en_passant = m_en_passant;
    st.castle_rights = m_castle_rights;
    st.halfmove = m_halfmove;
    m_history_ply++;

    Square from = static_cast<Square>(move.from());
    Square to = static_cast<Square>(move.to());
    uint16_t move_type = move.type();
    
    // Find the piece that's moving (check board, not move type)
    PieceType pt = NONE;
    for (int p = 0; p < NUM_PIECES; ++p) {
        if (m_pieces[p][m_side] & (1ULL << from)) {
            pt = static_cast<PieceType>(p);
            break;
        }
    }
    
    Color opponent = static_cast<Color>(1 - m_side);
    bool is_capture = move.is_capture();
    bool is_promotion = move.is_promotion();
    bool is_castling = move.is_castle();
    bool is_en_passant = move.is_en_passant();
    bool is_double_push = move.is_double_push();

    // Handle en passant capture
    if (is_en_passant) {
        Square captured_sq = static_cast<Square>(to + (m_side == WHITE ? -8 : 8));
        clear_piece(captured_sq, PAWN, opponent);
        m_halfmove = 0;
    }
    // Handle regular captures
    else if (is_capture) {
        for (int cap_pt = 0; cap_pt < NUM_PIECES; ++cap_pt) {
            if (m_pieces[cap_pt][opponent] & (1ULL << to)) {
                clear_piece(to, static_cast<PieceType>(cap_pt), opponent);
                m_halfmove = 0;
                break;
            }
        }
    }

    // Move the piece
    clear_piece(from, pt, m_side);
    
    // Handle promotion
    if (is_promotion) {
        PieceType promo_piece = move.promotion_piece();
        place_piece(to, promo_piece, m_side);
    } else {
        place_piece(to, pt, m_side);
    }

    // Handle castling - move the rook
    if (is_castling) {
        if (move.is_kingside_castle()) {
            // Kingside: rook from h-file to f-file
            Square rook_from = (m_side == WHITE) ? SQ_H1 : SQ_H8;
            Square rook_to = (m_side == WHITE) ? SQ_F1 : SQ_F8;
            clear_piece(rook_from, ROOK, m_side);
            place_piece(rook_to, ROOK, m_side);
        } else {
            // Queenside: rook from a-file to d-file
            Square rook_from = (m_side == WHITE) ? SQ_A1 : SQ_A8;
            Square rook_to = (m_side == WHITE) ? SQ_D1 : SQ_D8;
            clear_piece(rook_from, ROOK, m_side);
            place_piece(rook_to, ROOK, m_side);
        }
    }

    // Update castling rights based on piece movements
    update_castling_rights(from, to);
    
    // King move invalidates all castling rights for that side
    if (pt == KING) {
        m_castle_rights &= (m_side == WHITE) ? 0b1100 : 0b0011;
    }

    // Update halfmove clock and en passant square
    if (pt == PAWN) {
        m_halfmove = 0;
        if (is_double_push) {
            // Set en passant square (the square the pawn passed through)
            Square ep_sq = static_cast<Square>((from + to) / 2);
            m_en_passant = ep_sq;
        } else {
            m_en_passant = 64;
        }
    } else {
        if (!is_capture) {
            m_halfmove++;
        }
        m_en_passant = 64;
    }

    // Switch side
    m_side = opponent;
    if (m_side == WHITE) {
        m_fullmove++;
    }
}

void Board::unmake_move(const Move& move) {
    m_history_ply--;
    State& st = m_history[m_history_ply];

    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        for (int c = 0; c < NUM_COLORS; ++c) {
            m_pieces[pt][c] = st.pieces[pt][c];
        }
    }
    for (int c = 0; c < NUM_COLORS; ++c) {
        m_colors[c] = st.colors[c];
    }
    m_side = st.side;
    m_en_passant = st.en_passant;
    m_castle_rights = st.castle_rights;
    m_halfmove = st.halfmove;
    m_fullmove = (m_side == WHITE) ? m_fullmove : m_fullmove - 1;
}

void Board::update_castling_rights(Square from, Square to) {
    // White kingside rook
    if (from == SQ_H1 || to == SQ_H1) m_castle_rights &= 0b1110;
    // White queenside rook
    if (from == SQ_A1 || to == SQ_A1) m_castle_rights &= 0b1101;
    // Black kingside rook
    if (from == SQ_H8 || to == SQ_H8) m_castle_rights &= 0b1011;
    // Black queenside rook
    if (from == SQ_A8 || to == SQ_A8) m_castle_rights &= 0b0111;
    // White king
    if (from == SQ_E1 || to == SQ_E1) m_castle_rights &= 0b1100;
    // Black king
    if (from == SQ_E8 || to == SQ_E8) m_castle_rights &= 0b0011;
}

std::string Board::to_fen() const {
    std::string fen;

    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(f, r);
            bool found = false;
            for (int pt = 0; pt < NUM_PIECES; ++pt) {
                for (int c = 0; c < NUM_COLORS; ++c) {
                    if (m_pieces[pt][c] & (1ULL << sq)) {
                        if (empty > 0) {
                            fen += std::to_string(empty);
                            empty = 0;
                        }
                        char piece_char = "PNBRQKpnbrqk"[pt + (c == BLACK ? 6 : 0)];
                        fen += piece_char;
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }
            if (!found) empty++;
        }
        if (empty > 0) fen += std::to_string(empty);
        if (r > 0) fen += '/';
    }

    fen += (m_side == WHITE) ? " w " : " b ";

    // Castling rights
    if (m_castle_rights == 0) fen += "-";
    else {
        if (m_castle_rights & 0b0001) fen += "K";
        if (m_castle_rights & 0b0010) fen += "Q";
        if (m_castle_rights & 0b0100) fen += "k";
        if (m_castle_rights & 0b1000) fen += "q";
    }

    fen += " ";

    // En passant
    if (m_en_passant == 64) {
        fen += "-";
    } else {
        char file = 'a' + file_of(m_en_passant);
        char rank = '1' + rank_of(m_en_passant);
        fen += file;
        fen += rank;
    }

    fen += " " + std::to_string(m_halfmove) + " " + std::to_string(m_fullmove);
    return fen;
}
