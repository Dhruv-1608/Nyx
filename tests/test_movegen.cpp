#include <iostream>
#include "board.h"
#include "movegen.h"

void test_starting_position() {
    Board board;
    board.reset();

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    std::cout << "Starting position: " << moves.size() << " legal moves" << std::endl;
    // Should be 20 legal moves (16 pawn moves + 4 knight moves)
    if (moves.size() == 20) {
        std::cout << "  PASS: Expected 20 moves" << std::endl;
    } else {
        std::cout << "  FAIL: Expected 20, got " << moves.size() << std::endl;
    }
}

void test_fools_mate() {
    Board board;
    board.load_fen("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 4");

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    std::cout << "Fool's mate position: " << moves.size() << " legal moves" << std::endl;
    // Black can checkmate with Qh2#
    bool can_checkmate = false;
    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];
        if (m.to() == 33) { // h2 square
            can_checkmate = true;
            break;
        }
    }

    if (can_checkmate) {
        std::cout << "  PASS: Checkmate move available" << std::endl;
    } else {
        std::cout << "  FAIL: Checkmate move not found" << std::endl;
    }
}

void test_castling_rights() {
    Board board;
    board.reset();

    std::cout << "Castling rights (startpos): ";
    if (board.can_castle(WHITE, true) && board.can_castle(WHITE, false) &&
        board.can_castle(BLACK, true) && board.can_castle(BLACK, false)) {
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
    }

    // After king moves, castling rights should be lost
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1");
    
    // Actually move the king to invalidate castling rights
    Move king_move;
    king_move.set_from(4); // e1
    king_move.set_to(5);   // e2
    king_move.set_piece(KING);
    board.make_move(king_move);
    
    std::cout << "Castling rights (king moved): ";
    if (!board.can_castle(WHITE, true) && !board.can_castle(WHITE, false)) {
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
    }
}

void test_en_passant() {
    Board board;
    board.load_fen("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 3");

    std::cout << "En passant target: ";
    if (board.en_passant() == 40) { // c6
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL (expected c6, got " << board.en_passant() << ")" << std::endl;
    }

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    // Should have en passant capture available
    bool has_ep = false;
    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];
        if (m.to() == 40 && m.from() == 28) { // d5 pawn captures on c6
            has_ep = true;
            break;
        }
    }

    if (has_ep) {
        std::cout << "  En passant move found: PASS" << std::endl;
    } else {
        std::cout << "  En passant move missing: FAIL" << std::endl;
    }
}

void test_promotion() {
    Board board;
    board.load_fen("8/P7/8/8/8/8/8/4K2k w - - 0 1");

    MoveGenerator mg(board);
    MoveList moves = mg.generate_all();

    bool has_promotion = false;
    for (size_t i = 0; i < moves.size(); ++i) {
        Move m = moves[i];
        if (m.is_promotion() && m.to() == 8) { // a7 pawn promotes to a8
            has_promotion = true;
            break;
        }
    }

    std::cout << "Promotion test: ";
    if (has_promotion) {
        std::cout << "PASS" << std::endl;
    } else {
        std::cout << "FAIL" << std::endl;
    }
}

int main() {
    std::cout << "=== Nyx Move Generator Tests ===" << std::endl;

    test_starting_position();
    test_fools_mate();
    test_castling_rights();
    test_en_passant();
    test_promotion();

    std::cout << "\nTests complete." << std::endl;
    return 0;
}
