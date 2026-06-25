#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "transposition.h"
#include <memory>
#include <vector>
#include <array>
#include <chrono>

class Searcher {
public:
    Searcher();
    ~Searcher() = default;
    struct Config {
        int max_depth;
        int64_t max_time;
        bool use_tt;
    };
    int search(Board& board, Move& best_move, const Config& config);
    void iterative_deepening(Board& board, Move& best_move, const Config& config);
    struct Stats {
        uint64_t nodes;
        uint64_t qnodes;          // Quiescence search nodes
        uint64_t tt_hits;
        uint64_t tt_probes;
        uint64_t tt_cutoffs;      // Cutoffs from TT probe
        uint64_t cutoffs;         // Beta cutoffs in main search
        uint64_t repetitions;     // Repetitions detected
        int64_t time_ms;
    };
    Stats get_stats() const { return m_stats; }
    void reset_stats();
    void stop() { m_stop_search = true; }
    void add_history(uint64_t hash) { m_position_history.push_back(hash); }
    void clear_history() { m_position_history.clear(); }

    // Access the principal variation line
    const std::vector<Move>& pv_line() const { return m_pv_line; }

private:
    std::unique_ptr<TranspositionTable> m_tt;
    Evaluator m_eval;
    Config m_config;
    Stats m_stats;
    int m_best_score;
    std::vector<uint64_t> m_position_history;
    std::chrono::steady_clock::time_point m_start_time;
    
    // Principal Variation collection
    static constexpr int MAX_PLIES = 128;
    Move m_pv_table[MAX_PLIES][MAX_PLIES]; // [depth][ply]
    int m_pv_length[MAX_PLIES];
    std::vector<Move> m_pv_line;            // Best PV line from root

    int alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move, int ply = 0);
    int quiescence(Board& board, int alpha, int beta, int depth, int ply = 0);
    int aspiration_search(Board& board, int depth, Move& best_move);
    void order_moves(MoveList& moves, const Board& board, Move tt_move = Move());
    int see_capture(const Board& board, Square to, PieceType capturer) const;
    int stand_pat(const Board& board) const;
    bool check_time();
    bool m_stop_search;
    PieceType find_lva(const Board& board, Bitboard attackers, Color stm) const;
    Square find_attacker_square(const Board& board, Bitboard attackers, Color stm) const;

    bool is_capture(const Move& move, const Board& board) const;
    bool has_non_pawn_material(const Board& board, Color c) const;
    int see(const Board& board, Square sq, int threshold) const;
    bool is_repetition(const Board& board) const;
};

#endif
