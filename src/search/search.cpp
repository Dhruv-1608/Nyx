#include "search.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "transposition.h"
#include <algorithm>
#include <chrono>
#include <cmath>

using namespace std;

Searcher::Searcher() : m_tt(std::make_unique<TranspositionTable>(16)) {
    reset_stats();
}

void Searcher::reset_stats() {
    m_stats = {0, 0, 0, 0};
}

int Searcher::search(Board& board, Move& best_move, const Config& config) {
    m_config = config;
    reset_stats();

    auto start_time = std::chrono::steady_clock::now();

    if (config.max_depth == 1) {
        int score = alpha_beta(board, 1, -30000, 30000, false);
        MoveGenerator mg(board);
        MoveList moves = mg.generate_all();
        if (moves.size() > 0) {
            best_move = moves[0];
        }
        return score;
    }

    iterative_deepening(board, best_move, config);

    auto end_time = std::chrono::steady_clock::now();
    m_stats.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    return m_eval.evaluate(board);
}

void Searcher::iterative_deepening(Board& board, Move& best_move, const Config& config) {
    Move current_best;
    int best_score = -30000;

    for (int depth = 1; depth <= config.max_depth; ++depth) {
        int score = alpha_beta(board, depth, -30000, 30000, false);
        if (score > best_score || depth == 1) {
            best_score = score;
            current_best = best_move;
        }
        if (config.max_time > 0 && m_stats.time_ms >= config.max_time) {
            break;
        }
    }

    best_move = current_best;
}

int Searcher::alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null) {
    m_stats.nodes++;

    // TT probe
    if (m_config.use_tt) {
        uint64_t key = compute_zobrist(board);
        int tt_score;
        Move tt_move;
        if (m_tt->probe(key, depth, alpha, beta, tt_score, tt_move)) {
            m_stats.tt_probes++;
            if (tt_score >= beta) {
                return beta;
            }
            if (tt_score > alpha) {
                alpha = tt_score;
            }
        }
    }

    if (depth == 0) {
        return quiescence(board, alpha, beta, 0);
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    if (moves.size() == 0) {
        Color us = board.side_to_move();
        Square ksq = board.find_king(us);
        if (ksq != 64) {
            Bitboard attacks = mg.attacks_to_square(ksq, static_cast<Color>(1 - us));
            if (attacks & board.all_pieces(us)) {
                return -30000 + (int)board.halfmove_clock();
            }
        }
        return 0;
    }

    order_moves(moves, board);

    int best_score = -30000;
    Move best_move_local;

    for (size_t i = 0; i < moves.size(); ++i) {
        Move move = moves[i];

        board.make_move(move);

        int score;
        if (i == 0) {
            score = -alpha_beta(board, depth - 1, -beta, -alpha, true);
        } else {
            // Late move reductions
            int reduction = (depth >= 3 && i >= 4) ? 1 : 0;
            int new_depth = depth - 1 - reduction;
            score = -alpha_beta(board, new_depth, -alpha - 1, -alpha, false);
            if (score > alpha && reduction > 0) {
                score = -alpha_beta(board, depth - 1, -beta, -alpha, false);
            }
        }

        board.unmake_move(move);

        if (score > best_score) {
            best_score = score;
            best_move_local = move;
        }
        if (score > alpha) {
            alpha = score;
        }
        if (alpha >= beta) {
            if (m_config.use_tt) {
                uint64_t key = compute_zobrist(board);
                m_tt->store(key, best_score, depth, LOWERBOUND, best_move_local);
            }
            break;
        }
    }

    // Store in transposition table
    if (m_config.use_tt) {
        uint64_t key = compute_zobrist(board);
        TTFlag flag = (best_score <= alpha) ? UPPERBOUND : EXACT;
        m_tt->store(key, best_score, depth, flag, best_move_local);
    }

    return best_score;
}

int Searcher::quiescence(Board& board, int alpha, int beta, int depth) {
    constexpr int MAX_QUIESCENCE_DEPTH = 8;
    if (depth >= MAX_QUIESCENCE_DEPTH) {
        return m_eval.evaluate(board);
    }

    m_stats.nodes++;

    int stand_pat_score = m_eval.evaluate(board);
    if (stand_pat_score >= beta) {
        return beta;
    }
    if (stand_pat_score > alpha) {
        alpha = stand_pat_score;
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_pseudo_legal();

    for (size_t i = 0; i < moves.size(); ++i) {
        Move move = moves[i];
        Square to = static_cast<Square>(move.to());
        if (!(board.all_pieces() & (1ULL << to))) {
            continue;
        }

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

void Searcher::order_moves(MoveList& moves, const Board& board, Move tt_move) {
    (void)moves;
    (void)board;
    (void)tt_move;
}

bool Searcher::see_capture(const Board& board, Square to, PieceType capturer) const {
    return true;
}

int Searcher::stand_pat(const Board& board) const {
    return m_eval.evaluate(board);
}

bool Searcher::is_capture(const Move& move, const Board& board) const {
    Square to = static_cast<Square>(move.to());
    return (board.all_pieces() & (1ULL << to)) != 0;
}
