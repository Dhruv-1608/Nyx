#include "cli.h"
#include "board.h"
#include "search.h"
#include "move_validator.h"
#include "types.h"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>

CLI::CLI() {
    m_board = std::make_unique<Board>();
    m_searcher = std::make_unique<Searcher>();
    m_board->reset();
    m_searcher->clear_history();
    m_searcher->add_history(m_board->zobrist_key());
}

CLI::~CLI() = default;

void CLI::run() {
    std::cout << "Nyx Chess Engine - Command Line Interface" << std::endl;
    std::cout << "Enter moves in UCI format (e.g., e2e4, e7e5, g1f3)" << std::endl;
    std::cout << "Commands: 'go' (engine moves), 'fen' (show FEN), 'quit'" << std::endl;

    while (true) {
        print_board();
        std::cout << (m_board->side_to_move() == WHITE ? "White" : "Black") << " to move: ";
        std::string input;
        
        std::getline(std::cin, input);
        
        // Check for input failure or EOF
        if (std::cin.fail() || std::cin.eof()) {
            std::cout << "\nInput stream closed. Exiting." << std::endl;
            break;
        }

        // Clean input first (remove whitespace and lowercase)
        input.erase(std::remove_if(input.begin(), input.end(), ::isspace), input.end());
        std::transform(input.begin(), input.end(), input.begin(), ::tolower);

        if (input == "quit" || input == "exit") break;
        if (input == "go") {
            make_engine_move(5);
            continue;
        }
        if (input == "fen") {
            std::cout << m_board->to_fen() << std::endl;
            continue;
        }
        if (input.empty()) continue;

        Move move;
        if (parse_move(input, move)) {
            if (MoveValidator::is_legal(*m_board, move)) {
                m_board->make_move(move);
                m_searcher->add_history(m_board->zobrist_key());
            } else {
                std::cout << "Illegal move" << std::endl;
            }
        } else {
            std::cout << "Invalid move format" << std::endl;
        }
    }
}

void CLI::print_board() const {
    std::cout << "\n   a b c d e f g h" << std::endl;
    std::cout << "  +-----------------+" << std::endl;
    for (int r = 7; r >= 0; --r) {
        std::cout << r+1 << " |";
        for (int f = 0; f < 8; ++f) {
            Square sq = make_square(f, r);
            char piece = '.';
            for (int pt = 0; pt < NUM_PIECES; ++pt) {
                if (m_board->pieces(static_cast<PieceType>(pt), WHITE) & (1ULL << sq)) {
                    piece = "PNBRQK"[pt];
                    break;
                }
                if (m_board->pieces(static_cast<PieceType>(pt), BLACK) & (1ULL << sq)) {
                    piece = "pnbrqk"[pt];
                    break;
                }
            }
            std::cout << piece << ' ';
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "  +-----------------+" << std::endl;
    std::cout << "   a b c d e f g h" << std::endl;
}

void CLI::make_engine_move(int depth) {
    Searcher::Config config;
    config.max_depth = depth;
    config.max_time = 0;
    config.use_tt = true;

    Move best_move;
    m_searcher->search(*m_board, best_move, config);
    m_board->make_move(best_move);
    m_searcher->add_history(m_board->zobrist_key());
    std::cout << "Engine plays: " << move_to_string(best_move) << std::endl;
}

std::string CLI::move_to_string(const Move& move) const {
    Square from = static_cast<Square>(move.from());
    Square to = static_cast<Square>(move.to());
    std::string s;
    s += 'a' + file_of(from);
    s += '1' + rank_of(from);
    s += 'a' + file_of(to);
    s += '1' + rank_of(to);
    if (move.is_promotion()) {
        PieceType pt = move.promotion_piece();
        if (pt == QUEEN) s += 'q';
        else if (pt == ROOK) s += 'r';
        else if (pt == BISHOP) s += 'b';
        else if (pt == KNIGHT) s += 'n';
    }
    return s;
}

bool CLI::parse_move(const std::string& str, Move& move) const {
    if (str.length() < 4) return false;
    int from_file = str[0] - 'a';
    int from_rank = str[1] - '1';
    int to_file = str[2] - 'a';
    int to_rank = str[3] - '1';
    if (from_file < 0 || from_file > 7 || from_rank < 0 || from_rank > 7 ||
        to_file < 0 || to_file > 7 || to_rank < 0 || to_rank > 7) {
        return false;
    }
    Square from = make_square(from_file, from_rank);
    Square to = make_square(to_file, to_rank);
    move.set_from(from);
    move.set_to(to);
    
    Color us = m_board->side_to_move();
    Color opponent = static_cast<Color>(1 - us);
    if (m_board->all_pieces(opponent) & (1ULL << to)) {
        move.set_type(CAPTURE);
    } else {
        move.set_type(QUIET);
    }

    if (str.length() > 4) {
        char prom = str[4];
        PieceType pt = QUEEN;
        if (prom == 'n') pt = KNIGHT;
        else if (prom == 'b') pt = BISHOP;
        else if (prom == 'r') pt = ROOK;
        move.set_promotion(pt, move.is_capture());
    }
    return true;
}
