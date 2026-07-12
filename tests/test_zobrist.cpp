#include <iostream>
#include <string>
#include <cstdlib>
#include "board.h"
#include "movegen.h"
#include "move_validator.h"
#include "zobrist.h"

static int tests_passed = 0;
static int tests_failed = 0;

static void check(bool condition, const char* name) {
    std::cout << "  " << name << "... ";
    if (condition) {
        std::cout << "PASS" << std::endl;
        tests_passed++;
    } else {
        std::cout << "FAIL" << std::endl;
        tests_failed++;
    }
}

// Apply a move by UCI string, return the Move object
static Move parse_uci(Board& board, const std::string& uci) {
    Move move;
    int from_file = uci[0] - 'a';
    int from_rank = uci[1] - '1';
    int to_file = uci[2] - 'a';
    int to_rank = uci[3] - '1';
    Square from = make_square(from_file, from_rank);
    Square to = make_square(to_file, to_rank);
    move.set_from(from);
    move.set_to(to);

    Color us = board.side_to_move();
    Color opponent = static_cast<Color>(1 - us);

    if (board.all_pieces(opponent) & (1ULL << to)) {
        move.set_type(CAPTURE);
    } else {
        move.set_type(QUIET);
    }

    if (from == SQ_E1 && to == SQ_G1 && us == WHITE) move.set_type(CASTLE_KS);
    if (from == SQ_E1 && to == SQ_C1 && us == WHITE) move.set_type(CASTLE_QS);
    if (from == SQ_E8 && to == SQ_G8 && us == BLACK) move.set_type(CASTLE_KS);
    if (from == SQ_E8 && to == SQ_C8 && us == BLACK) move.set_type(CASTLE_QS);

    PieceType pt = NONE;
    for (int p = PAWN; p <= KING; ++p) {
        if (board.pieces(static_cast<PieceType>(p), us) & (1ULL << from)) {
            pt = static_cast<PieceType>(p);
            break;
        }
    }
    if (pt == PAWN && abs(static_cast<int>(to) - static_cast<int>(from)) == 16) {
        move.set_type(DOUBLE_PUSH);
    }
    if (board.en_passant() == to && pt == PAWN) {
        move.set_type(EN_PASSANT);
    }
    if (uci.length() == 5) {
        char prom = uci[4];
        PieceType promo = QUEEN;
        if (prom == 'n') promo = KNIGHT;
        else if (prom == 'b') promo = BISHOP;
        else if (prom == 'r') promo = ROOK;
        bool is_cap = move.is_capture();
        move.set_promotion(promo, is_cap);
    }

    return move;
}

// Verify the board's incremental zobrist key matches a FEN round-trip
static bool verify_key(const Board& board) {
    std::string fen = board.to_fen();
    Board fresh;
    if (!fresh.load_fen(fen)) return false;
    return board.zobrist_key() == fresh.zobrist_key();
}

// Apply a move, verify incremental key during make/unmake cycle
static bool verify_single_move(const std::string& fen, const std::string& uci) {
    Board board;
    if (!board.load_fen(fen)) return false;

    std::string fen_before = board.to_fen();
    uint64_t key_before = board.zobrist_key();

    Move move = parse_uci(board, uci);
    board.make_move(move);

    // Verify key after move matches FEN round-trip
    if (!verify_key(board)) return false;

    board.unmake_move(move);

    // Verify key and FEN restored
    if (board.zobrist_key() != key_before) return false;
    if (board.to_fen() != fen_before) return false;

    return true;
}

// Apply a sequence of moves permanently, verify final key
static bool verify_sequence(const std::string& start_fen,
                            const std::string* moves, int num_moves) {
    Board board;
    if (!board.load_fen(start_fen)) return false;

    for (int i = 0; i < num_moves; ++i) {
        Move move = parse_uci(board, moves[i]);
        board.make_move(move);
    }

    return verify_key(board);
}

// Make a sequence, track each state, unmake in reverse
static bool verify_full_cycle(const std::string& start_fen,
                               const std::string* moves, int num_moves) {
    Board board;
    if (!board.load_fen(start_fen)) return false;

    struct State {
        Move move;
        std::string uci;
        std::string fen;
        uint64_t key;
    };
    std::vector<State> history;

    State init;
    init.fen = board.to_fen();
    init.key = board.zobrist_key();

    for (int i = 0; i < num_moves; ++i) {
        State st;
        st.uci = moves[i];
        st.fen = board.to_fen();
        st.key = board.zobrist_key();

        if (!verify_key(board)) return false;

        st.move = parse_uci(board, moves[i]);
        board.make_move(st.move);

        if (!verify_key(board)) return false;

        history.push_back(st);
    }

    for (int i = num_moves - 1; i >= 0; --i) {
        board.unmake_move(history[i].move);
        if (board.zobrist_key() != history[i].key) return false;
        if (board.to_fen() != history[i].fen) return false;
        if (!verify_key(board)) return false;
    }

    if (board.zobrist_key() != init.key) return false;
    if (board.to_fen() != init.fen) return false;

    return true;
}

// ============ TESTS ============

void test_startpos_key() {
    Board board;
    board.reset();
    check(verify_key(board), "Starting position key matches FEN round-trip");
}

void test_quiet_moves() {
    check(verify_single_move("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e2e4"),
          "e2e4 quiet move");
    check(verify_single_move("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "d2d4"),
          "d2d4 quiet move");
    check(verify_single_move("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "g1f3"),
          "g1f3 knight move");
}

void test_captures() {
    check(verify_single_move("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3", "d1h5"),
          "Queen takes pawn capture");
    check(verify_single_move("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3", "g1f3"),
          "Nf3 development");
}

void test_castling() {
    // Open files for castling: remove bishops, knights, queens from relevant squares
    // FEN: r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1
    // Both sides can castle in all four directions.
    check(verify_single_move("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", "e1g1"),
          "White kingside castle (O-O)");
    check(verify_single_move("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1", "e1c1"),
          "White queenside castle (O-O-O)");
    check(verify_single_move("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1", "e8g8"),
          "Black kingside castle (O-O)");
    check(verify_single_move("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R b KQkq - 0 1", "e8c8"),
          "Black queenside castle (O-O-O)");
}

void test_en_passant() {
    check(verify_single_move("rnbqkbnr/ppp2ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 3", "d5e6"),
          "En passant capture - white");
    check(verify_single_move("rnbqkbnr/pppp1ppp/8/8/4PpPp/8/PPPP2PP/RNBQKBNR b KQkq e3 0 3", "f4e3"),
          "En passant capture - black");
}

void test_double_push() {
    check(verify_single_move("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "e2e4"),
          "White double push e2e4");
    Board b; b.reset();
    Move m = parse_uci(b, "e2e4"); b.make_move(m);
    std::string fen = b.to_fen();
    check(verify_single_move(fen, "d7d5"), "Black double push d7d5");
}

void test_promotions() {
    check(verify_single_move("8/P7/8/8/8/8/8/k6K w - - 0 1", "a7a8q"),
          "White queen promotion");
    check(verify_single_move("K6k/8/8/8/8/8/p7/8 b - - 0 1", "a2a1n"),
          "Black knight promotion");
}

void test_null_moves() {
    auto test_null = [](const std::string& fen) -> bool {
        Board board;
        if (!board.load_fen(fen)) return false;
        std::string fen_before = board.to_fen();
        uint64_t key_before = board.zobrist_key();

        board.make_null_move();
        if (!verify_key(board)) return false;

        board.unmake_null_move();
        if (board.zobrist_key() != key_before) return false;
        if (board.to_fen() != fen_before) return false;
        return true;
    };

    check(test_null("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
          "Null move from startpos");
    check(test_null("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1"),
          "Null move from open position");

    // Null move after setting en passant target
    Board b; b.reset();
    Move m = parse_uci(b, "e2e4"); b.make_move(m);
    std::string fen = b.to_fen();
    check(test_null(fen), "Null move with en passant target set");
}

void test_sequences() {
    std::string moves1[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5"};
    check(verify_sequence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        moves1, 5), "Ruy Lopez opening: 1.e4 e5 2.Nf3 Nc6 3.Bb5");

    std::string moves2[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6", "d2d4", "e5d4", "e4e5"};
    check(verify_sequence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        moves2, 9), "Italian game with captures");

    // Castling sequence: clear bishops/knights for open lines
    std::string moves3[] = {"e1g1", "e8g8", "f1e1", "f8e8"};
    check(verify_sequence(
        "r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1",
        moves3, 4), "Castling sequence (O-O O-O)");

    std::string moves4[] = {
        "e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "f8c5",
        "e1g1", "g8f6", "f1e1", "e8g8", "b1c3", "d7d6", "d2d4"
    };
    check(verify_sequence(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        moves4, 13), "Long Italian game sequence (13 plies)");
}

void test_make_unmake_full_cycle() {
    std::string moves[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "a7a6"};
    check(verify_full_cycle(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        moves, 6), "Make/unmake full cycle (6 plies)");
}

void test_key_uniqueness() {
    Board b1, b2;
    b1.reset();
    b2.reset();
    Move m = parse_uci(b2, "e2e4");
    b2.make_move(m);
    check(b1.zobrist_key() != b2.zobrist_key(),
          "Different positions have different keys");

    Board b3; b3.reset();
    m = parse_uci(b3, "d2d4"); b3.make_move(m);
    m = parse_uci(b3, "d7d5"); b3.make_move(m);
    m = parse_uci(b3, "d1d5"); b3.make_move(m);

    Board b4; b4.reset();
    m = parse_uci(b4, "d2d4"); b4.make_move(m);
    m = parse_uci(b4, "d7d5"); b4.make_move(m);
    m = parse_uci(b4, "d1d5"); b4.make_move(m);

    check(b3.zobrist_key() == b4.zobrist_key(),
          "Same position reached via same path has identical key");
}

int main() {
    std::cout << "=== Nyx Zobrist Key Incremental Update Tests ===" << std::endl;
    std::cout << "Verifies the incremental zobrist key matches FEN round-trip" << std::endl;
    std::cout << "after every move type: quiet, capture, castle, en passant," << std::endl;
    std::cout << "double push, promotion, and null move." << std::endl;
    std::cout << std::endl;

    Zobrist::init_keys();

    test_startpos_key();
    test_quiet_moves();
    test_captures();
    test_castling();
    test_en_passant();
    test_double_push();
    test_promotions();
    test_null_moves();
    test_sequences();
    test_make_unmake_full_cycle();
    test_key_uniqueness();

    std::cout << std::endl;
    std::cout << "=== Results: " << tests_passed << " passed, " << tests_failed << " failed ===" << std::endl;

    return tests_failed > 0 ? 1 : 0;
}
