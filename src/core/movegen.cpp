#include "movegen.h"
#include "board.h"

namespace {
    constexpr int knight_dx[8] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    constexpr int knight_dy[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
    constexpr int king_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    constexpr int king_dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    constexpr int pawn_attack_dx[2][2] = { { -1, 1 }, { -1, 1 } };
    constexpr int bishop_dirs[4] = { -9, -7, 7, 9 };
    constexpr int rook_dirs[4] = { -8, -1, 1, 8 };

    inline bool on_board(int x, int y) {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }

    inline Square to_square(int x, int y) {
        return static_cast<Square>(y * 8 + x);
    }
}

MoveGenerator::MoveGenerator(const Board& board) : m_board(board) {}

MoveList MoveGenerator::generate_all() const {
    MoveList list;
    MoveList pseudo = generate_pseudo_legal();

    Color us = m_board.side_to_move();
    Square our_king = m_board.find_king(us);
    if (our_king == 64) return list; // No king, invalid position

    for (const Move& move : pseudo.moves) {
        Board temp = m_board;
        temp.make_move(move);
        Color opponent = static_cast<Color>(1 - us);
        Square our_king_after = temp.find_king(us);
        if (our_king_after == 64) continue;
        Bitboard opponent_attacks = attacks_to_square(our_king_after, opponent);
        if (opponent_attacks & (1ULL << our_king_after)) {
            continue;
        }
        list.add_move(move);
    }

    return list;
}

MoveList MoveGenerator::generate_pseudo_legal() const {
    MoveList list;

    Color us = m_board.side_to_move();
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        Bitboard bb = m_board.pieces(static_cast<PieceType>(pt), us);
        while (bb) {
            Square sq = static_cast<Square>(__builtin_ctzll(bb));
            bb &= bb - 1;
            switch (pt) {
                case PAWN: gen_pawn_moves(sq, us, list); break;
                case KNIGHT: gen_knight_moves(sq, us, list); break;
                case BISHOP: gen_bishop_moves(sq, us, list); break;
                case ROOK: gen_rook_moves(sq, us, list); break;
                case QUEEN: gen_queen_moves(sq, us, list); break;
                case KING: gen_king_moves(sq, us, list); break;
                default: break;
            }
        }
    }

    return list;
}

bool MoveGenerator::is_pseudo_legal(const Move& move) const {
    MoveList pseudo = generate_pseudo_legal();
    for (const Move& m : pseudo.moves) {
        if (m.from() == move.from() && m.to() == move.to()) {
            return true;
        }
    }
    return false;
}

void MoveGenerator::gen_pawn_moves(Square sq, Color c, MoveList& list) const {
    int x = file_of(sq);
    int y = rank_of(sq);
    int direction = (c == WHITE) ? 1 : -1;
    Color opponent = static_cast<Color>(1 - c);
    
    // Forward moves
    Square to = to_square(x, y + direction);
    if (on_board(x, y + direction) && !(m_board.all_pieces() & (1ULL << to))) {
        if ((c == WHITE && y + direction == 7) || (c == BLACK && y + direction == 0)) {
            for (int prom = QUEEN; prom >= KNIGHT; --prom) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(static_cast<PieceType>(prom));
                m.set_promotion(1);
                list.add_move(m);
            }
        } else {
            Move m;
            m.set_from(sq);
            m.set_to(to);
            m.set_piece(PAWN);
            list.add_move(m);
        }
        if ((c == WHITE && y == 1) || (c == BLACK && y == 6)) {
            Square to2 = to_square(x, y + 2 * direction);
            if (!(m_board.all_pieces() & (1ULL << to2))) {
                Move m2;
                m2.set_from(sq);
                m2.set_to(to2);
                m2.set_piece(PAWN);
                list.add_move(m2);
            }
        }
    }

    // Captures
    for (int d = 0; d < 2; ++d) {
        int nx = x + pawn_attack_dx[c][d];
        int ny = y + direction;
        if (on_board(nx, ny)) {
            Square target = to_square(nx, ny);
            if (m_board.all_pieces(opponent) & (1ULL << target)) {
                if ((c == WHITE && ny == 7) || (c == BLACK && ny == 0)) {
                    for (int prom = QUEEN; prom >= KNIGHT; --prom) {
                        Move m;
                        m.set_from(sq);
                        m.set_to(target);
                        m.set_piece(static_cast<PieceType>(prom));
                        m.set_promotion(1);
                        list.add_move(m);
                    }
                } else {
                    Move m;
                    m.set_from(sq);
                    m.set_to(target);
                    m.set_piece(PAWN);
                    list.add_move(m);
                }
            }

            // En passant
            if (m_board.en_passant() == target) {
                Move m;
                m.set_from(sq);
                m.set_to(target);
                m.set_piece(PAWN);
                list.add_move(m);
            }
        }
    }
}

void MoveGenerator::gen_knight_moves(Square sq, Color c, MoveList& list) const {
    int x = file_of(sq);
    int y = rank_of(sq);
    Color opponent = static_cast<Color>(1 - c);
    Bitboard their_pieces = m_board.all_pieces(opponent);
    Bitboard empty = ~m_board.all_pieces();

    for (int i = 0; i < 8; ++i) {
        int nx = x + knight_dx[i];
        int ny = y + knight_dy[i];
        if (on_board(nx, ny)) {
            Square to = to_square(nx, ny);
            Bitboard to_bb = 1ULL << to;
            if (empty & to_bb) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(KNIGHT);
                list.add_move(m);
            } else if (their_pieces & to_bb) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(KNIGHT);
                list.add_move(m);
            }
        }
    }
}

void MoveGenerator::gen_bishop_moves(Square sq, Color c, MoveList& list) const {
    int x = file_of(sq);
    int y = rank_of(sq);
    Color opponent = static_cast<Color>(1 - c);
    Bitboard their_pieces = m_board.all_pieces(opponent);
    Bitboard all_occ = m_board.all_pieces();

    for (int dir = 0; dir < 4; ++dir) {
        int dx = (dir < 2) ? (dir == 0 ? -1 : 1) : 0;
        int dy = (dir >= 2) ? (dir == 2 ? 1 : -1) : 0;

        int nx = x + dx;
        int ny = y + dy;
        while (on_board(nx, ny)) {
            Square to = to_square(nx, ny);
            Bitboard to_bb = 1ULL << to;
            if (!(all_occ & to_bb)) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(BISHOP);
                list.add_move(m);
            } else {
                if (their_pieces & to_bb) {
                    Move m;
                    m.set_from(sq);
                    m.set_to(to);
                    m.set_piece(BISHOP);
                    list.add_move(m);
                }
                break;
            }
            nx += dx;
            ny += dy;
        }
    }
}

void MoveGenerator::gen_rook_moves(Square sq, Color c, MoveList& list) const {
    int x = file_of(sq);
    int y = rank_of(sq);
    Color opponent = static_cast<Color>(1 - c);
    Bitboard their_pieces = m_board.all_pieces(opponent);
    Bitboard all_occ = m_board.all_pieces();

    const int dirs[4][2] = { {0, 1}, {0, -1}, {1, 0}, {-1, 0} };

    for (int d = 0; d < 4; ++d) {
        int nx = x + dirs[d][0];
        int ny = y + dirs[d][1];
        while (on_board(nx, ny)) {
            Square to = to_square(nx, ny);
            Bitboard to_bb = 1ULL << to;
            if (!(all_occ & to_bb)) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(ROOK);
                list.add_move(m);
            } else {
                if (their_pieces & to_bb) {
                    Move m;
                    m.set_from(sq);
                    m.set_to(to);
                    m.set_piece(ROOK);
                    list.add_move(m);
                }
                break;
            }
            nx += dirs[d][0];
            ny += dirs[d][1];
        }
    }
}

void MoveGenerator::gen_queen_moves(Square sq, Color c, MoveList& list) const {
    gen_bishop_moves(sq, c, list);
    gen_rook_moves(sq, c, list);
}

void MoveGenerator::gen_king_moves(Square sq, Color c, MoveList& list) const {
    int x = file_of(sq);
    int y = rank_of(sq);
    Color opponent = static_cast<Color>(1 - c);
    Bitboard their_pieces = m_board.all_pieces(opponent);
    Bitboard empty = ~m_board.all_pieces();

    // Normal king moves
    for (int i = 0; i < 8; ++i) {
        int nx = x + king_dx[i];
        int ny = y + king_dy[i];
        if (on_board(nx, ny)) {
            Square to = to_square(nx, ny);
            Bitboard to_bb = 1ULL << to;
            if (empty & to_bb) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(KING);
                list.add_move(m);
            } else if (their_pieces & to_bb) {
                Move m;
                m.set_from(sq);
                m.set_to(to);
                m.set_piece(KING);
                list.add_move(m);
            }
        }
    }

    // Castling
    if (m_board.can_castle(c, true)) {
        // Kingside castling
        if (c == WHITE) {
            if (!(m_board.all_pieces() & (1ULL << SQ_F1)) &&
                !(m_board.all_pieces() & (1ULL << SQ_G1))) {
                Move m;
                m.set_from(SQ_E1);
                m.set_to(SQ_G1);
                m.set_piece(KING);
                list.add_move(m);
            }
        } else {
            if (!(m_board.all_pieces() & (1ULL << SQ_F8)) &&
                !(m_board.all_pieces() & (1ULL << SQ_G8))) {
                Move m;
                m.set_from(SQ_E8);
                m.set_to(SQ_G8);
                m.set_piece(KING);
                list.add_move(m);
            }
        }
    }

    if (m_board.can_castle(c, false)) {
        // Queenside castling
        if (c == WHITE) {
            if (!(m_board.all_pieces() & (1ULL << SQ_D1)) &&
                !(m_board.all_pieces() & (1ULL << SQ_C1)) &&
                !(m_board.all_pieces() & (1ULL << SQ_B1))) {
                Move m;
                m.set_from(SQ_E1);
                m.set_to(SQ_C1);
                m.set_piece(KING);
                list.add_move(m);
            }
        } else {
            if (!(m_board.all_pieces() & (1ULL << SQ_D8)) &&
                !(m_board.all_pieces() & (1ULL << SQ_C8)) &&
                !(m_board.all_pieces() & (1ULL << SQ_B8))) {
                Move m;
                m.set_from(SQ_E8);
                m.set_to(SQ_C8);
                m.set_piece(KING);
                list.add_move(m);
            }
        }
    }
}

Bitboard MoveGenerator::attacks_to_square(Square sq, Color attacker) const {
    Bitboard attacks = 0;
    int x = file_of(sq);
    int y = rank_of(sq);

    // Pawn attacks
    int pawn_dir = (attacker == WHITE) ? 1 : -1;
    if (x > 0) attacks |= 1ULL << to_square(x - 1, y + pawn_dir);
    if (x < 7) attacks |= 1ULL << to_square(x + 1, y + pawn_dir);

    // Knight attacks
    for (int i = 0; i < 8; ++i) {
        int nx = x + knight_dx[i];
        int ny = y + knight_dy[i];
        if (on_board(nx, ny)) {
            attacks |= 1ULL << to_square(nx, ny);
        }
    }

    // King attacks
    for (int i = 0; i < 8; ++i) {
        int nx = x + king_dx[i];
        int ny = y + king_dy[i];
        if (on_board(nx, ny)) {
            attacks |= 1ULL << to_square(nx, ny);
        }
    }

    // Sliding pieces
    Bitboard occ = m_board.all_pieces();

    // Bishop/Queen diagonal attacks
    const int bishop_dirs[4] = { -9, -7, 7, 9 };
    for (int dir : bishop_dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            attacks |= 1ULL << s;
            if (occ & (1ULL << s)) break;
            s += dir;
            // Wrap check
            if (dir == -7 && (s % 8 == 7)) break;
            if (dir == 7 && (s % 8 == 0)) break;
            if (dir == -9 && (s % 8 == 0)) break;
            if (dir == 9 && (s % 8 == 7)) break;
        }
    }

    // Rook/Queen orthogonal attacks
    const int rook_dirs[4] = { -8, -1, 1, 8 };
    for (int dir : rook_dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            attacks |= 1ULL << s;
            if (occ & (1ULL << s)) break;
            s += dir;
            // Prevent file wrap
            if (dir == -1 && (s % 8 == 7)) break;
            if (dir == 1 && (s % 8 == 0)) break;
        }
    }

    return attacks;
}

Bitboard MoveGenerator::bishop_moves_bb(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    const int bishop_dirs[4] = { -9, -7, 7, 9 };
    for (int dir : bishop_dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            attacks |= 1ULL << s;
            if (occupied & (1ULL << s)) break;
            s += dir;
            if (dir == -7 && (s % 8 == 7)) break;
            if (dir == 7 && (s % 8 == 0)) break;
            if (dir == -9 && (s % 8 == 0)) break;
            if (dir == 9 && (s % 8 == 7)) break;
        }
    }
    return attacks;
}

Bitboard MoveGenerator::rook_moves_bb(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    const int rook_dirs[4] = { -8, -1, 1, 8 };
    for (int dir : rook_dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            attacks |= 1ULL << s;
            if (occupied & (1ULL << s)) break;
            s += dir;
            if (dir == -1 && (s % 8 == 7)) break;
            if (dir == 1 && (s % 8 == 0)) break;
        }
    }
    return attacks;
}

Bitboard MoveGenerator::queen_moves_bb(Square sq, Bitboard occupied) const {
    return bishop_moves_bb(sq, occupied) | rook_moves_bb(sq, occupied);
}

Square MoveGenerator::king_square(Color c) const {
    return m_board.find_king(c);
}

bool MoveGenerator::in_check(Color c) const {
    Square ksq = king_square(c);
    if (ksq == 64) return false;
    Color opponent = static_cast<Color>(1 - c);
    return attacks_to_square(ksq, opponent) & m_board.all_pieces(c);
}
