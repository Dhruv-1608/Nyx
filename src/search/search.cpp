#include "search.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "transposition.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <iostream>

using namespace std;

Searcher::Searcher() : m_tt(std::make_unique<TranspositionTable>(16)), m_best_score(0), m_stop_search(false) {
    reset_stats();
}

void Searcher::reset_stats() {
    m_stats = {0, 0, 0, 0};
}

bool Searcher::has_non_pawn_material(const Board& board, Color c) const {
    return (board.all_pieces(c) ^ board.pieces(PAWN, c) ^ board.pieces(KING, c)) != 0;
}

void Searcher::check_time() {
    // Only check every few nodes to reduce overhead
    if ((m_stats.nodes & 2047) == 0) {
        // Implementation for time checking would go here
    }
}

int Searcher::search(Board& board, Move& best_move, const Config& config) {
    m_config = config;
    reset_stats();

    auto start_time = std::chrono::steady_clock::now();

    // Use iterative deepening to find the best move
    int score = 0;
    for (int depth = 1; depth <= m_config.max_depth; ++depth) {
        Move current_best_move;
        score = alpha_beta(board, depth, -30000, 30000, true, current_best_move);
        
        if (current_best_move.is_valid()) {
            best_move = current_best_move;
        }

        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
        if (m_config.max_time > 0 && elapsed > m_config.max_time) {
            break;
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    m_stats.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    return score;
}

void Searcher::iterative_deepening(Board& board, Move& best_move, const Config& config) {
    search(board, best_move, config);
}

int Searcher::alpha_beta_internal(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move) {
    best_move = Move();
    m_stats.nodes++;

    if (depth <= 0) {
        return quiescence(board, alpha, beta, 0);
    }

    bool in_check = board.in_check(board.side_to_move());

    // Null move pruning
    if (do_null && depth >= 3 && !in_check && has_non_pawn_material(board, board.side_to_move())) {
        board.make_null_move();
        int R = 2 + depth / 6;
        Move dummy;
        int null_score = -alpha_beta_internal(board, depth - 1 - R, -beta, -beta + 1, false, dummy);
        board.unmake_null_move();

        if (null_score >= beta) {
            return beta;
        }
    }

    // Futility pruning
    if (depth <= 2 && !in_check) {
        int stand_pat_score = stand_pat(board);
        if (stand_pat_score >= beta + 150) {
            return beta;
        }
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();
    
    if (moves.size() == 0) {
        Color us = board.side_to_move();
        Square ksq = board.find_king(us);
        if (ksq != 64) {
            Bitboard attacks = mg.attacks_to_square(ksq, static_cast<Color>(1 - us));
            if (attacks) {
                return -30000 + (int)board.halfmove_clock();
            }
        }
        return 0;
    }

    // Move ordering
    order_moves(moves, board);

    int best_score = -30000;
    Move best_move_local;

    for (const Move& move : moves) {
        board.make_move(move);
        Move dummy;
        int score = -alpha_beta_internal(board, depth - 1, -beta, -alpha, true, dummy);
        board.unmake_move(move);

        if (score > best_score) {
            best_score = score;
            best_move_local = move;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            break;
        }
    }

    best_move = best_move_local;
    return best_score;
}

int Searcher::alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move) {
    // Aspiration windows
    if (depth >= 3) {
        int prev_score = 0; // Simplified for now, should use TT
        int delta = 35; // Initial aspiration delta
        
        // Search with aspiration window
        int new_alpha = std::max(-30000, alpha - delta);
        int new_beta = std::min(30000, beta + delta);
        
        // Try multiple times with widening window
        for (int i = 0; i < 4; ++i) {
            int score = alpha_beta_internal(board, depth, new_alpha, new_beta, do_null, best_move);
            
            if (score <= new_alpha) {
                new_beta = (new_alpha + new_beta) / 2;
                new_alpha = std::max(-30000, new_alpha - delta);
                delta *= 2;
            } else if (score >= new_beta) {
                new_beta = std::min(30000, new_beta + delta);
                delta *= 2;
            } else {
                return score; // Exact score found
            }
        }
    }
    // Fall back to full window search
    return alpha_beta_internal(board, depth, alpha, beta, do_null, best_move);
}

int Searcher::quiescence(Board& board, int alpha, int beta, int depth) {
    m_stats.nodes++;
    int stand_pat_score = stand_pat(board);
    
    if (stand_pat_score >= beta) {
        return beta;
    }

    // Delta pruning
    if (stand_pat_score < alpha - 300 - depth * 50) {
        return alpha;
    }

    if (alpha < stand_pat_score) {
        alpha = stand_pat_score;
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();
    
    // Only search captures in quiescence
    MoveList captures;
    for (const Move& move : moves) {
        if (is_capture(move, board)) {
            captures.add_move(move);
        }
    }

    order_moves(captures, board);

    for (const Move& move : captures) {
        board.make_move(move);
        int score = -quiescence(board, -beta, -alpha, depth + 1);
        board.unmake_move(move);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

PieceType Searcher::find_lva(const Board& board, Bitboard attackers, Color stm) const {
    for (int pt = PAWN; pt <= KING; ++pt) {
        if (attackers & board.pieces(static_cast<PieceType>(pt), stm)) {
            return static_cast<PieceType>(pt);
        }
    }
    return NONE;
}

Square Searcher::find_attacker_square(const Board& board, Bitboard attackers, Color stm) const {
    for (int pt = PAWN; pt <= KING; ++pt) {
        Bitboard b = attackers & board.pieces(static_cast<PieceType>(pt), stm);
        if (b) {
            return static_cast<Square>(pop_lsb(b)); // Use pop_lsb and cast
        }
    }
    return 64; // Invalid square
}

int Searcher::see(const Board& board, Square sq, int threshold) const {
    int gain[32];
    int d = 0;

    Board current_board = board;
    Color stm = current_board.side_to_move();
    
    gain[d] = MG_PIECE_VALUE[current_board.piece_at(sq)];

    Bitboard occupied = current_board.all_pieces();

    while (true) {
        d++;
        stm = static_cast<Color>(1 - stm);

        MoveGenerator mg(current_board);
        Bitboard attackers = mg.attacks_to_square(sq, stm) & occupied;

        PieceType attacker_piece = find_lva(current_board, attackers, stm);
        if (attacker_piece == NONE) {
            break;
        }

        Square attacker_sq = find_attacker_square(current_board, attackers, stm);
        
        gain[d] = MG_PIECE_VALUE[attacker_piece];

        current_board.remove_piece(attacker_sq, attacker_piece, stm);
        occupied ^= (1ULL << attacker_sq);
    }

    // Minimax on the gain array
    for (int i = d - 2; i >= 0; --i) {
        gain[i] = -std::max(-gain[i], gain[i+1]);
    }
    return gain[0];
}

int Searcher::see_capture(const Board& board, Square to, PieceType capturer) const {
    // Get value of captured piece
    PieceType captured_piece = board.piece_at(to);
    if (captured_piece == NONE) return 0; // Should not happen for a capture

    int capture_value = MG_PIECE_VALUE[captured_piece];
    
    // Get value of attacker
    int attacker_value = MG_PIECE_VALUE[capturer];
    
    // If not a winning capture, return 0 early for now, will be improved with full SEE
    if (attacker_value >= capture_value) return 1; // Placeholder for winning capture

    // Perform SEE
    // For now, only return a basic indication. Full SEE will be implemented in the see function above.
    return 0; // Placeholder for losing capture
}

int Searcher::stand_pat(const Board& board) const {
    return m_eval.evaluate(board);
}

bool Searcher::is_capture(const Move& move, const Board& board) const {
    Square to = static_cast<Square>(move.to());
    return board.piece_at(to) != NONE;
}

void Searcher::order_moves(MoveList& moves, const Board& board, Move tt_move) {
    struct ScoredMove {
        Move move;
        int score;
    };
    
    std::vector<ScoredMove> scored;
    scored.reserve(moves.size());
    
    for (const Move& move : moves) {
        int score = 0;
        Square to = static_cast<Square>(move.to());
        Square from = static_cast<Square>(move.from());
        
        // TT move gets highest priority
        if (tt_move.from() == move.from() && tt_move.to() == move.to()) {
            score = 20000;
        }
        // Winning captures
        else if (is_capture(move, board)) {
            score = 10000 + see(board, to, 0); // Use the new SEE function
        }
        // Promotions
        else if (move.is_promotion()) {
            score = 8000 + MG_PIECE_VALUE[move.promotion_piece()];
        }
        // Quiet moves - use history heuristic (not implemented yet, default to 0)
        else {
            score = 0; 
        }
        
        scored.push_back({move, score});
    }
    
    // Sort descending
    std::sort(scored.begin(), scored.end(), 
              [](const ScoredMove& a, const ScoredMove& b) { 
                  return a.score > b.score; 
              });
    
    // Write back to move list
    for (size_t i = 0; i < scored.size(); ++i) {
        moves.moves[i] = scored[i].move;
    }
}