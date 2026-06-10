#include "search.h"
#include "../core/board.h"
#include "../core/movegen.h"
#include "../core/eval.h"
#include "transposition.h"
#include <algorithm>
#include <chrono>
#include <iostream>

using namespace std;

Searcher::Searcher() : m_tt(std::make_unique<TranspositionTable>(16)), m_best_score(0), m_stop_search(false) {
    reset_stats();
}

void Searcher::reset_stats() {
    m_stats = {0, 0, 0, 0};
}

bool Searcher::is_repetition(const Board& board) const {
    uint64_t current_hash = board.zobrist_key();
    int count = 0;
    for (uint64_t h : m_position_history) {
        if (h == current_hash) count++;
    }
    return count >= 2;
}

int Searcher::search(Board& board, Move& best_move, const Config& config) {
    m_config = config;
    reset_stats();
    m_stop_search = false;

    Move current_best;
    int score = alpha_beta(board, config.max_depth, -30000, 30000, true, current_best);
    best_move = current_best;
    return score;
}

int Searcher::alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move) {
    m_stats.nodes++;
    
    if (is_repetition(board)) return 0;
    if (depth <= 0) return m_eval.evaluate(board);

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();
    
    if (moves.size() == 0) {
        if (mg.in_check(board.side_to_move())) return -20000 - depth;
        return 0;
    }

    int best_score = -30000;
    Move local_best = moves[0];

    for (const Move& move : moves.moves) {
        board.make_move(move);
        m_position_history.push_back(board.zobrist_key());
        
        Move dummy;
        int score = -alpha_beta(board, depth - 1, -beta, -alpha, true, dummy);
        
        m_position_history.pop_back();
        board.unmake_move(move);

        if (score > best_score) {
            best_score = score;
            local_best = move;
        }
        alpha = max(alpha, best_score);
        if (alpha >= beta) break;
    }

    best_move = local_best;
    return best_score;
}
