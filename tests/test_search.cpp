#include <iostream>
#include "board.h"
#include "search.h"

void test_basic_search() {
    std::cerr << "=== test_basic_search START ===" << std::endl;
    Board board;
    board.reset();

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 3;
    config.use_tt = true;

    Move best_move;
    int score = searcher.search(board, best_move, config);

    std::cerr << "test_basic_search: score=" << score << " move.data=" << best_move.data << std::endl;
    std::cout << "Basic search (depth 3): score = " << score << std::endl;
    if (best_move.data != 0) {
        std::cout << "  Best move: " << best_move.from() << "->" << best_move.to() << std::endl;
        std::cout << "  PASS: Search completed" << std::endl;
    } else {
        std::cout << "  FAIL: No move found" << std::endl;
    }
    std::cerr << "=== test_basic_search END ===" << std::endl;
}

void test_search_stats() {
    Board board;
    board.reset();

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 2;
    config.use_tt = false;

    Move best_move;
    searcher.search(board, best_move, config);

    Searcher::Stats stats = searcher.get_stats();
    std::cout << "Search statistics:" << std::endl;
    std::cout << "  Nodes: " << stats.nodes << std::endl;
    std::cout << "  Time: " << stats.time_ms << "ms" << std::endl;

    if (stats.nodes > 0) {
        std::cout << "  PASS: Nodes searched" << std::endl;
    } else {
        std::cout << "  FAIL: No nodes searched" << std::endl;
    }
}

void test_iterative_deepening() {
    Board board;
    board.reset();

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 4;
    config.use_tt = true;

    Move best_move;
    int score = searcher.search(board, best_move, config);

    std::cout << "Iterative deepening (depth 4): score = " << score << std::endl;
    if (best_move.data != 0) {
        std::cout << "  PASS: Iterative deepening works" << std::endl;
    } else {
        std::cout << "  FAIL: No move found" << std::endl;
    }
}

void test_transposition_table() {
    Board board;
    board.reset();

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 3;
    config.use_tt = true;

    Move best_move1;
    int score1 = searcher.search(board, best_move1, config);

    // Search same position again - should hit TT
    Move best_move2;
    int score2 = searcher.search(board, best_move2, config);

    Searcher::Stats stats = searcher.get_stats();
    std::cout << "Transposition table test:" << std::endl;
    std::cout << "  TT probes: " << stats.tt_probes << std::endl;

    if (score1 == score2) {
        std::cout << "  PASS: Consistent results" << std::endl;
    } else {
        std::cout << "  FAIL: Inconsistent results" << std::endl;
    }
}

void test_checkmate_detection() {
    Board board;
    board.load_fen("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 4");

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 2;
    config.use_tt = false;

    Move best_move;
    int score = searcher.search(board, best_move, config);

    std::cout << "Checkmate detection test:" << std::endl;
    std::cout << "  Score: " << score << std::endl;
    // Should detect that Black can checkmate (negative score for White)
    if (score < -20000) {
        std::cout << "  PASS: Checkmate detected" << std::endl;
    } else {
        std::cout << "  FAIL: Checkmate not detected (score=" << score << ")" << std::endl;
    }
}

void test_stalemate_detection() {
    Board board;
    // Stalemate position: King trapped but not in check
    board.load_fen("8/8/8/8/8/5k2/5p2/5K2 w - - 0 1");

    Searcher searcher;
    Searcher::Config config;
    config.max_depth = 2;
    config.use_tt = false;

    Move best_move;
    int score = searcher.search(board, best_move, config);

    std::cout << "Stalemate test:" << std::endl;
    std::cout << "  Score: " << score << std::endl;
    // Stalemate should be 0
    if (abs(score) < 100) {
        std::cout << "  PASS: Stalemate detected" << std::endl;
    } else {
        std::cout << "  FAIL: Expected ~0, got " << score << std::endl;
    }
}

int main() {
    std::cout << "=== Nyx Search Tests ===" << std::endl;

    test_basic_search();
    test_search_stats();
    test_iterative_deepening();
    test_transposition_table();
    test_checkmate_detection();
    test_stalemate_detection();

    std::cout << "\nTests complete." << std::endl;
    return 0;
}
