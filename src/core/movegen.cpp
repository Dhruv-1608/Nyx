#include "movegen.h"
#include "board.h"
#include "bitboards.h"
#include <iostream>

using namespace Bitboards;

namespace {
    constexpr int knight_dx[8] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    constexpr int knight_dy[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
    constexpr int king_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    constexpr int king_dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    constexpr int pawn_attack_dx[2][2] = { { -1, 1 }, { -1, 1 } };
}

MoveGenerator::MoveGenerator(const Board& board) : m_board(board) {}

MoveList MoveGenerator::generate_all() const {
    MoveList list;
    MoveList pseudo = generate_pseudo_legal();

    Color us = m_board.side_to_move();
    
    for (const Move& move : pseudo.moves) {
        if (!(m_board.all_pieces(us) & (1ULL << move.from()))) continue;
        
        Board temp = m_board;
        temp.make_move(move);
        if (!MoveGenerator(temp).in_check(us)) {
            list.add_move(move);
        }
    }

    return list;
}

MoveList MoveGenerator::generate_pseudo_legal() const {
    MoveList list;
    Color us = m_board.side_to_move();
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        Bitboard bb = m_board.pieces(static_cast<PieceType>(pt), us);
        while (bb) {
            Square sq = static_cast<Square>(BitOps::ctzll(bb));
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
        if (m.from() == move.from() && m.to() == move.to()) return true;
    }
    return false;
}

void MoveGenerator::gen_pawn_moves(Square sq, Color c, MoveList& list) const {
    Bitboard us = 1ULL << sq;
    Bitboard all_occ = m_board.all_pieces();
    Bitboard their_pieces = m_board.all_pieces(static_cast<Color>(1 - c));
    int start_rank = (c == WHITE) ? 1 : 6;

    Bitboard push = (c == WHITE) ? (us << 8) : (us >> 8);
    if (!(push & all_occ)) {
        Square to = static_cast<Square>(BitOps::ctzll(push));
        if ((c == WHITE && rank_of(to) == 7) || (c == BLACK && rank_of(to) == 0)) {
            for (int prom = QUEEN; prom >= KNIGHT; --prom) {
                Move m; m.set_from(sq); m.set_to(to);
                m.set_promotion(static_cast<PieceType>(prom), false);
                list.add_move(m);
            }
        } else {
            Move m; m.set_from(sq); m.set_to(to); m.set_type(QUIET);
            list.add_move(m);
            Bitboard push2 = (c == WHITE) ? (push << 8) : (push >> 8);
            if (rank_of(sq) == start_rank && !(push2 & all_occ)) {
                Move m2; m2.set_from(sq); m2.set_to(static_cast<Square>(BitOps::ctzll(push2)));
                m2.set_type(DOUBLE_PUSH);
                list.add_move(m2);
            }
        }
    }

    Bitboard captures = PawnAttacks[c][sq] & their_pieces;
    while (captures) {
        Square to = static_cast<Square>(BitOps::ctzll(captures));
        captures &= captures - 1;
        if ((c == WHITE && rank_of(to) == 7) || (c == BLACK && rank_of(to) == 0)) {
            for (int prom = QUEEN; prom >= KNIGHT; --prom) {
                Move m; m.set_from(sq); m.set_to(to);
                m.set_promotion(static_cast<PieceType>(prom), true);
                list.add_move(m);
            }
        } else {
            Move m; m.set_from(sq); m.set_to(to); m.set_type(CAPTURE);
            list.add_move(m);
        }
    }

    if (m_board.en_passant() != 64) {
        if (PawnAttacks[c][sq] & (1ULL << m_board.en_passant())) {
            Move m; m.set_from(sq); m.set_to(m_board.en_passant());
            m.set_type(EN_PASSANT);
            list.add_move(m);
        }
    }
}

void MoveGenerator::gen_knight_moves(Square sq, Color c, MoveList& list) const {
    Bitboard attacks = KnightAttacks[sq] & ~m_board.all_pieces(c);
    while (attacks) {
        Square to = static_cast<Square>(BitOps::ctzll(attacks));
        attacks &= attacks - 1;
        Move m; m.set_from(sq); m.set_to(to);
        m.set_type((m_board.all_pieces(static_cast<Color>(1-c)) & (1ULL << to)) ? CAPTURE : QUIET);
        list.add_move(m);
    }
}

void MoveGenerator::gen_bishop_moves(Square sq, Color c, MoveList& list) const {
    Bitboard attacks = bishop_moves_bb(sq, m_board.all_pieces()) & ~m_board.all_pieces(c);
    while (attacks) {
        Square to = static_cast<Square>(BitOps::ctzll(attacks));
        attacks &= attacks - 1;
        Move m; m.set_from(sq); m.set_to(to);
        m.set_type((m_board.all_pieces(static_cast<Color>(1-c)) & (1ULL << to)) ? CAPTURE : QUIET);
        list.add_move(m);
    }
}

void MoveGenerator::gen_rook_moves(Square sq, Color c, MoveList& list) const {
    Bitboard attacks = rook_moves_bb(sq, m_board.all_pieces()) & ~m_board.all_pieces(c);
    while (attacks) {
        Square to = static_cast<Square>(BitOps::ctzll(attacks));
        attacks &= attacks - 1;
        Move m; m.set_from(sq); m.set_to(to);
        m.set_type((m_board.all_pieces(static_cast<Color>(1-c)) & (1ULL << to)) ? CAPTURE : QUIET);
        list.add_move(m);
    }
}

void MoveGenerator::gen_queen_moves(Square sq, Color c, MoveList& list) const {
    gen_bishop_moves(sq, c, list);
    gen_rook_moves(sq, c, list);
}

void MoveGenerator::gen_king_moves(Square sq, Color c, MoveList& list) const {
    Bitboard attacks = KingAttacks[sq] & ~m_board.all_pieces(c);
    while (attacks) {
        Square to = static_cast<Square>(BitOps::ctzll(attacks));
        attacks &= attacks - 1;
        Move m; m.set_from(sq); m.set_to(to);
        m.set_type((m_board.all_pieces(static_cast<Color>(1-c)) & (1ULL << to)) ? CAPTURE : QUIET);
        list.add_move(m);
    }

    if (m_board.can_castle(c, true)) {
        if (c == WHITE && !(m_board.all_pieces() & (1ULL << SQ_F1 | 1ULL << SQ_G1)) && !in_check(WHITE) && !(attacks_to_square(SQ_F1, BLACK) & m_board.all_pieces(BLACK))) {
            Move m; m.set_from(SQ_E1); m.set_to(SQ_G1); m.set_type(CASTLE_KS); list.add_move(m);
        } else if (c == BLACK && !(m_board.all_pieces() & (1ULL << SQ_F8 | 1ULL << SQ_G8)) && !in_check(BLACK) && !(attacks_to_square(SQ_F8, WHITE) & m_board.all_pieces(WHITE))) {
            Move m; m.set_from(SQ_E8); m.set_to(SQ_G8); m.set_type(CASTLE_KS); list.add_move(m);
        }
    }
    if (m_board.can_castle(c, false)) {
        if (c == WHITE && !(m_board.all_pieces() & (1ULL << SQ_D1 | 1ULL << SQ_C1 | 1ULL << SQ_B1)) && !in_check(WHITE) && !(attacks_to_square(SQ_D1, BLACK) & m_board.all_pieces(BLACK))) {
            Move m; m.set_from(SQ_E1); m.set_to(SQ_C1); m.set_type(CASTLE_QS); list.add_move(m);
        } else if (c == BLACK && !(m_board.all_pieces() & (1ULL << SQ_D8 | 1ULL << SQ_C8 | 1ULL << SQ_B8)) && !in_check(BLACK) && !(attacks_to_square(SQ_D8, WHITE) & m_board.all_pieces(WHITE))) {
            Move m; m.set_from(SQ_E8); m.set_to(SQ_C8); m.set_type(CASTLE_QS); list.add_move(m);
        }
    }
}

Bitboard MoveGenerator::attacks_to_square(Square sq, Color attacker) const {
    Bitboard occ = m_board.all_pieces();
    return (PawnAttacks[static_cast<Color>(1-attacker)][sq] & m_board.pieces(PAWN, attacker)) |
           (KnightAttacks[sq] & m_board.pieces(KNIGHT, attacker)) |
           (KingAttacks[sq] & m_board.pieces(KING, attacker)) |
           (bishop_moves_bb(sq, occ) & (m_board.pieces(BISHOP, attacker) | m_board.pieces(QUEEN, attacker))) |
           (rook_moves_bb(sq, occ) & (m_board.pieces(ROOK, attacker) | m_board.pieces(QUEEN, attacker)));
}

bool MoveGenerator::in_check(Color c) const {
    Square ksq = m_board.find_king(c);
    if (ksq == 64) return false;
    return attacks_to_square(ksq, static_cast<Color>(1 - c)) != 0;
}

Bitboard MoveGenerator::bishop_moves_bb(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    const int dirs[4] = { -9, -7, 7, 9 };
    for (int dir : dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            if (dir == -7 && (s % 8 == 0)) break;
            if (dir == 7 && (s % 8 == 7)) break;
            if (dir == -9 && (s % 8 == 7)) break;
            if (dir == 9 && (s % 8 == 0)) break;
            attacks |= 1ULL << s;
            if (occupied & (1ULL << s)) break;
            s += dir;
        }
    }
    return attacks;
}

Bitboard MoveGenerator::rook_moves_bb(Square sq, Bitboard occupied) const {
    Bitboard attacks = 0;
    const int dirs[4] = { -8, -1, 1, 8 };
    for (int dir : dirs) {
        int s = static_cast<int>(sq) + dir;
        while (s >= 0 && s < 64) {
            if (dir == -1 && (s % 8 == 7)) break;
            if (dir == 1 && (s % 8 == 0)) break;
            attacks |= 1ULL << s;
            if (occupied & (1ULL << s)) break;
            s += dir;
        }
    }
    return attacks;
}

Bitboard MoveGenerator::queen_moves_bb(Square sq, Bitboard occupied) const {
    return bishop_moves_bb(sq, occupied) | rook_moves_bb(sq, occupied);
}
