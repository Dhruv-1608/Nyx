#include "search.h"
#include "../core/board.h"
#include "../core/movegen.h"
#include "../core/eval.h"
#include "../core/types.h"
#include "transposition.h"
#include <algorithm>
#include <chrono>

using namespace std;

// MVV-LVA scores: victim - attacker
static const int MVV_LVA[6][6] = {
    {0,   0,   0,   0,   0,   0},   // Pawn capturing
    {0,   0,  10,  20,  30,  40},   // Knight capturing
    {0,  10,   0,  15,  25,  35},   // Bishop capturing
    {0,  20,  15,   0,  20,  30},   // Rook capturing
    {0,  30,  25,  20,   0,  10},   // Queen capturing
    {0,  40,  35,  30,  10,   0}    // King capturing
};

static int mvv_lva_score(const Board& board, const Move& move) {
    Square from = move.from();
    Square to = move.to();
    
    Color us = board.side_to_move();
    Color them = static_cast<Color>(1 - us);
    
    // Get attacker piece type
    PieceType attacker = NONE;
    for (int pt = PAWN; pt <= KING; ++pt) {
        if (board.pieces(static_cast<PieceType>(pt), us) & (1ULL << from)) {
            attacker = static_cast<PieceType>(pt);
            break;
        }
    }
    
    // Get victim piece type
    PieceType victim = NONE;
    for (int pt = PAWN; pt <= KING; ++pt) {
        if (board.pieces(static_cast<PieceType>(pt), them) & (1ULL << to)) {
            victim = static_cast<PieceType>(pt);
            break;
        }
    }
    
    if (victim != NONE && attacker != NONE) {
        return MVV_LVA[victim][attacker];
    }
    
    // Promotion bonus
    if (move.is_promotion()) {
        return 100;
    }
    
    return 0;
}

Searcher::Searcher() : m_tt(std::make_unique<TranspositionTable>(16)), m_best_score(0), m_stop_search(false) {
    reset_stats();
}

bool Searcher::is_repetition(const Board& board) const {
    uint64_t current_hash = board.zobrist_key();
    int count = 0;
    for (uint64_t h : m_position_history) {
        if (h == current_hash) count++;
    }
    return count >= 2;
}

void Searcher::reset_stats() {
    m_stats = {0, 0, 0, 0, 0, 0, 0, 0};
    // Clear PV tables
    for (int i = 0; i < MAX_PLIES; ++i) {
        m_pv_length[i] = 0;
        for (int j = 0; j < MAX_PLIES; ++j) {
            m_pv_table[i][j] = Move();
        }
    }
    m_pv_line.clear();
}

int Searcher::search(Board& board, Move& best_move, const Config& config) {
    m_config = config;
    reset_stats();
    m_start_time = std::chrono::steady_clock::now();
    m_stop_search = false;

    int prev_score = 0;
    for (int depth = 1; depth <= config.max_depth; ++depth) {
        if (check_time()) break;
        int score = aspiration_search(board, depth, best_move, prev_score);
        m_best_score = score;
        prev_score = score;

        // Extract PV line at this depth
        m_pv_line.clear();
        for (int i = 0; i < m_pv_length[0]; ++i) {
            m_pv_line.push_back(m_pv_table[0][i]);
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    m_stats.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - m_start_time).count();

    return m_best_score;
}

bool Searcher::check_time() {
    if (m_config.max_time <= 0) return false;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
    if (elapsed >= m_config.max_time) {
        m_stop_search = true;
        return true;
    }
    return false;
}

int Searcher::aspiration_search(Board& board, int depth, Move& best_move, int prev_score) {
    int window = 50;
    int alpha = std::max(prev_score - window, -30000);
    int beta  = std::min(prev_score + window,  30000);
    
    int score;
    int max_attempts = 3;
    
    for (int attempt = 0; attempt < max_attempts; ++attempt) {
        score = alpha_beta(board, depth, alpha, beta, false, best_move, 0);
        
        if (score <= alpha) {
            alpha = std::max(score - window, -30000);
            beta  = (alpha + beta) / 2;
            window *= 2;
        } else if (score >= beta) {
            beta  = std::min(score + window, 30000);
            alpha = (alpha + beta) / 2;
            window *= 2;
        } else {
            return score;
        }
    }
    return score;
}

void Searcher::iterative_deepening(Board& board, Move& best_move, const Config& config) {
    Move current_best;
    int best_score = -30000;

    for (int depth = 1; depth <= config.max_depth; ++depth) {
        int score = alpha_beta(board, depth, -30000, 30000, false, current_best, 0);

        if (score > best_score || depth == 1) {
            best_score = score;
            best_move = current_best;
            m_best_score = best_score;

            // Extract PV line
            m_pv_line.clear();
            for (int i = 0; i < m_pv_length[0]; ++i) {
                m_pv_line.push_back(m_pv_table[0][i]);
            }
        }

        if (config.max_time > 0 && m_stats.time_ms >= config.max_time) {
            break;
        }
    }
}

int Searcher::alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move, int ply) {
    if (depth >= 3 && do_null && !board.in_check(board.side_to_move()) && has_non_pawn_material(board, board.side_to_move())) {
        board.make_null_move();
        m_position_history.push_back(board.zobrist_key());
        Move dummy;
        int s = -alpha_beta(board, depth - 1 - 2, -beta, -beta + 1, false, dummy, ply + 1);
        m_position_history.pop_back();
        board.unmake_null_move();
        if (s >= beta) return beta;
    }
    m_stats.nodes++;

    if (m_stop_search) return 0;

    // Repetition detection
    if (is_repetition(board)) {
        m_stats.repetitions++;
        return 0;
    }

    // Initialize PV length for this node
    m_pv_length[ply] = 0;

    // Transposition table probe
    Move tt_move;
    if (m_config.use_tt) {
        uint64_t key = board.zobrist_key();
        int tt_score;
        if (m_tt->probe(key, depth, alpha, beta, tt_score, tt_move)) {
            m_stats.tt_probes++;
            m_stats.tt_hits++;
            if (tt_score >= beta) {
                m_stats.tt_cutoffs++;
                best_move = tt_move;
                return beta;
            }
            if (tt_score > alpha) {
                alpha = tt_score;
            }
        }
    }

    // Quiescence search at depth 0
    if (depth == 0) {
        return quiescence(board, alpha, beta, 0, ply);
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    if (moves.size() == 0) {
        if (board.in_check(board.side_to_move())) {
            return -30000 - depth;
        }
        return 0;
    }

    // Sort moves - TT move first, then by MVV-LVA
    vector<pair<int, Move>> scored_moves;
    for (const Move& move : moves.moves) {
        int score = mvv_lva_score(board, move);
        if (move == tt_move) score += 10000; // Prioritize TT move
        scored_moves.push_back({score, move});
    }
    sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    int best_score = -30000;
    Move local_best = scored_moves[0].second;

    for (const auto& [score, move] : scored_moves) {
        board.make_move(move);
        m_position_history.push_back(board.zobrist_key());
        
        Move dummy;
        int s = -alpha_beta(board, depth - 1, -beta, -alpha, true, dummy, ply + 1);
        
        m_position_history.pop_back();
        board.unmake_move(move);

        if (s > best_score) {
            best_score = s;
            local_best = move;

            // Update PV table: copy child's PV to this node's PV
            m_pv_table[ply][0] = move;
            for (int i = 0; i < m_pv_length[ply + 1]; ++i) {
                m_pv_table[ply][i + 1] = m_pv_table[ply + 1][i];
            }
            m_pv_length[ply] = m_pv_length[ply + 1] + 1;
        }
        alpha = max(alpha, best_score);
        if (alpha >= beta) {
            m_stats.cutoffs++;
            break;
        }
    }

    best_move = local_best;

    // Store in transposition table
    if (m_config.use_tt) {
        uint64_t key = board.zobrist_key();
        TTFlag flag = (best_score >= beta) ? LOWERBOUND : ((best_score <= alpha && alpha < beta - 1) ? UPPERBOUND : EXACT);
        m_tt->store(key, best_score, depth, flag, best_move);
    }

    return best_score;
}

int Searcher::quiescence(Board& board, int alpha, int beta, int depth, int ply) {
    (void)depth;
    m_stats.nodes++;
    m_stats.qnodes++;

    int stand_pat_score = m_eval.evaluate(board);
    if (stand_pat_score >= beta) {
        return beta;
    }
    if (stand_pat_score > alpha) {
        alpha = stand_pat_score;
    }

    MoveGenerator mg(board);

    // Generate legal captures only
    vector<pair<int, Move>> scored_moves;
    for (const Move& move : mg.generate_all().moves) {
        Square to = move.to();
        if (!(board.all_pieces() & (1ULL << to))) continue; // Not a capture
        scored_moves.push_back({mvv_lva_score(board, move), move});
    }
    sort(scored_moves.begin(), scored_moves.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });

    for (const auto& [score, move] : scored_moves) {
        board.make_move(move);
        int s = -quiescence(board, -beta, -alpha, depth + 1, ply + 1);
        board.unmake_move(move);

        if (s >= beta) return beta;
        if (s > alpha) alpha = s;
    }

    return alpha;
}

void Searcher::order_moves(MoveList& moves, const Board& board, Move tt_move) {
    (void)moves;
    (void)board;
    (void)tt_move;
}

int Searcher::see_capture(const Board& board, Square to, PieceType capturer) const {
    (void)board;
    (void)to;
    (void)capturer;
    return 0;
}

int Searcher::stand_pat(const Board& board) const {
    return m_eval.evaluate(board);
}

bool Searcher::is_capture(const Move& move, const Board& board) const {
    Square to = static_cast<Square>(move.to());
    return (board.all_pieces() & (1ULL << to)) != 0;
}

bool Searcher::has_non_pawn_material(const Board& board, Color c) const {
    for (int pt = KNIGHT; pt <= QUEEN; ++pt) {
        if (board.pieces(static_cast<PieceType>(pt), c) != 0) {
            return true;
        }
    }
    return false;
}

int Searcher::see(const Board& board, Square sq, int threshold) const {
    (void)board;
    (void)sq;
    (void)threshold;
    return 0;
}

PieceType Searcher::find_lva(const Board& board, Bitboard attackers, Color stm) const {
    (void)board;
    (void)attackers;
    (void)stm;
    return PAWN;
}

Square Searcher::find_attacker_square(const Board& board, Bitboard attackers, Color stm) const {
    (void)board;
    (void)attackers;
    (void)stm;
    return 0;
}
