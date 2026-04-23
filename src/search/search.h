#ifndef SEARCH_H
#define SEARCH_H

#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "transposition.h"
#include <memory>

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
        uint64_t tt_hits;
        uint64_t tt_probes;
        int64_t time_ms;
    };
    Stats get_stats() const { return m_stats; }
    void reset_stats();

private:
    std::unique_ptr<TranspositionTable> m_tt;
    Evaluator m_eval;
    Config m_config;
    Stats m_stats;
    int m_best_score;
    int alpha_beta(Board& board, int depth, int alpha, int beta, bool do_null, Move& best_move);
    int quiescence(Board& board, int alpha, int beta, int depth);
    void order_moves(MoveList& moves, const Board& board, Move tt_move = Move());
    int see_capture(const Board& board, Square to, PieceType capturer) const;
    int  stand_pat(const Board& board) const;
    void check_time();
    bool m_stop_search;
    PieceType find_lva(const Board& board, Bitboard attackers, Color stm) const;
    Square find_attacker_square(const Board& board, Bitboard attackers, Color stm) const;

    bool is_capture(const Move& move, const Board& board) const;
    bool has_non_pawn_material(const Board& board, Color c) const;
    int see(const Board& board, Square sq, int threshold) const;

};

#endif
