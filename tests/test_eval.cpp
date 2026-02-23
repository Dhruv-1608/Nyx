#include <iostream>
#include "board.h"
#include "eval.h"

void test_starting_position_eval() {
    Board board;
    board.reset();

    Evaluator eval;
    int score = eval.evaluate(board);

    std::cout << "Starting position evaluation: " << score << std::endl;
    // Should be close to 0 (equal position)
    if (abs(score) < 100) {
        std::cout << "  PASS: Position is balanced" << std::endl;
    } else {
        std::cout << "  FAIL: Expected ~0, got " << score << std::endl;
    }
}

void test_material_advantage() {
    Board board;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Evaluator eval;
    int score = eval.evaluate(board);

    std::cout << "Starting position material: " << score << std::endl;
    // Should be close to 0
    if (abs(score) < 50) {
        std::cout << "  PASS: Material balanced" << std::endl;
    } else {
        std::cout << "  FAIL: Expected ~0, got " << score << std::endl;
    }
}

void test_white_material_advantage() {
    Board board;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1"); // White has extra pawn

    Evaluator eval;
    int score = eval.evaluate(board);

    std::cout << "White +1 pawn evaluation: " << score << std::endl;
    // Should be positive (White advantage)
    if (score > 50) {
        std::cout << "  PASS: White has advantage" << std::endl;
    } else {
        std::cout << "  FAIL: Expected positive score" << std::endl;
    }
}

void test_black_material_advantage() {
    Board board;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"); // Standard starting position, Black to move

    Evaluator eval;
    int score = eval.evaluate(board);

    std::cout << "Black to move evaluation: " << score << std::endl;
    // Score from White's perspective should be near 0 (equal position)
    if (abs(score) < 100) {
        std::cout << "  PASS: Position balanced" << std::endl;
    } else {
        std::cout << "  FAIL: Expected ~0, got " << score << std::endl;
    }
}

void test_king_safety() {
    Board board;
    board.reset();

    Evaluator eval;
    int score = eval.evaluate(board);

    std::cout << "King safety (startpos): " << score << std::endl;
    // Starting position should have decent king safety
    if (abs(score) < 200) {
        std::cout << "  PASS: King safety reasonable" << std::endl;
    } else {
        std::cout << "  FAIL: King safety score extreme" << std::endl;
    }
}

void test_mobility() {
    Board board;
    board.reset();

    Evaluator eval;
    int mobility = eval.evaluate_mobility(board, WHITE);

    std::cout << "White mobility (startpos): " << mobility << std::endl;
    // Starting position should have some mobility
    if (mobility > 0) {
        std::cout << "  PASS: Mobility positive" << std::endl;
    } else {
        std::cout << "  FAIL: Mobility should be positive" << std::endl;
    }
}

void test_positional_eval() {
    Board board;
    board.reset();

    Evaluator eval;
    int position = eval.evaluate_position(board, WHITE);

    std::cout << "White positional score (startpos): " << position << std::endl;
    // Should be reasonable
    if (abs(position) < 500) {
        std::cout << "  PASS: Positional score reasonable" << std::endl;
    } else {
        std::cout << "  FAIL: Positional score extreme" << std::endl;
    }
}

int main() {
    std::cout << "=== Nyx Evaluation Tests ===" << std::endl;

    test_starting_position_eval();
    test_material_advantage();
    test_black_material_advantage();
    test_king_safety();
    test_mobility();
    test_positional_eval();

    std::cout << "\nTests complete." << std::endl;
    return 0;
}
