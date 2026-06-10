#include "uci.h"
#include "board.h"
#include "search.h"
#include "eval.h"
#include "movegen.h"
#include "move_validator.h"
#include "types.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

namespace {
    std::string trim(const std::string& value) {
        const std::string whitespace = " \t\r\n";
        const size_t start = value.find_first_not_of(whitespace);
        if (start == std::string::npos) return "";
        const size_t end = value.find_last_not_of(whitespace);
        return value.substr(start, end - start + 1);
    }
}

UCI::UCI() : m_debug_mode(false), m_should_stop(false) {
    m_searcher = std::make_unique<Searcher>();
}

UCI::~UCI() = default;

void UCI::run() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "uci") {
            cmd_uci();
        } else if (command == "isready") {
            cmd_isready();
        } else if (command == "position") {
            std::string args;
            std::getline(iss, args);
            cmd_position(trim(args));
        } else if (command == "go") {
            std::string args;
            std::getline(iss, args);
            cmd_go(trim(args));
        } else if (command == "stop") {
            cmd_stop();
        } else if (command == "quit") {
            cmd_quit();
            break;
        } else if (command == "debug") {
            std::string args;
            std::getline(iss, args);
            cmd_debug(trim(args));
        } else if (command == "setoption") {
            std::string args;
            std::getline(iss, args);
            cmd_setoption(trim(args));
        } else if (command == "d") {
            std::cout << m_board.to_fen() << std::endl;
        } else if (command == "eval") {
            Evaluator eval;
            int score = eval.evaluate(m_board);
            std::cout << "eval: " << score << std::endl;
        }
    }
}

void UCI::cmd_uci() {
    send_response("id name Nyx 1.0");
    send_response("id author Nyx Developer");
    send_response("option name Hash type spin default 16 min 1 max 1024");
    send_response("option name Threads type spin default 1 min 1 max 1");
    send_response("uciok");
}

void UCI::cmd_isready() {
    send_response("readyok");
}

void UCI::cmd_position(const std::string& args) {
    std::istringstream iss(args);
    std::string token;
    iss >> token;

    if (token == "startpos") {
        m_board.reset();
        m_searcher->clear_history();
        m_searcher->add_history(m_board.zobrist_key());
        if (iss >> token && token == "moves") {
            while (iss >> token) {
                Move move;
                if (parse_and_validate_move(token, move)) {
                    m_board.make_move(move);
                    m_searcher->add_history(m_board.zobrist_key());
                } else {
                    if (m_debug_mode) {
                        send_info("Invalid move received: " + token);
                    }
                    break;
                }
            }
        }
    } else if (token == "fen") {
        std::string fen;
        while (iss >> token && token != "moves") {
            fen += token + " ";
        }
        fen.pop_back();
        m_board.load_fen(fen);
        m_searcher->clear_history();
        m_searcher->add_history(m_board.zobrist_key());

        if (token == "moves") {
            while (iss >> token) {
                Move move;
                if (parse_and_validate_move(token, move)) {
                    m_board.make_move(move);
                    m_searcher->add_history(m_board.zobrist_key());
                } else {
                    if (m_debug_mode) {
                        send_info("Invalid move received: " + token);
                    }
                    break;
                }
            }
        }
    }
}

void UCI::cmd_go(const std::string& args) {
    m_should_stop = false;

    Searcher::Config config;
    config.max_depth = 3;
    config.max_time = 0;
    config.use_tt = true;

    std::istringstream iss(args);
    std::string token;
    while (iss >> token) {
        if (token == "depth") {
            iss >> config.max_depth;
            if (config.max_depth > 10) config.max_depth = 10;
        } else if (token == "movetime") {
            int ms;
            iss >> ms;
            config.max_time = ms;
            if (ms < 1000) config.max_depth = 2;
            else if (ms < 3000) config.max_depth = 3;
            else if (ms < 10000) config.max_depth = 5;
        } else if (token == "wtime" || token == "btime") {
            config.max_depth = 5;
        }
    }

    Move best_move;
    int score = m_searcher->search(m_board, best_move, config);

    Searcher::Stats stats = m_searcher->get_stats();

    if (m_debug_mode) {
        send_info("Score: " + std::to_string(score));
        send_info("Nodes: " + std::to_string(stats.nodes));
        send_info("Time: " + std::to_string(stats.time_ms) + "ms");
    }

    Move validated_move = MoveValidator::validate_and_fix(m_board, best_move);
    
    if (m_debug_mode) {
        MoveList legal_moves = MoveValidator::get_legal_moves(m_board);
        send_info("Legal moves count: " + std::to_string(legal_moves.size()));
        send_info("Search returned: " + move_to_uci(best_move));
        send_info("Validated move: " + move_to_uci(validated_move));
    }
    
    best_move = validated_move;

    send_best_move(move_to_uci(best_move));
}

void UCI::cmd_stop() {
    m_should_stop = true;
}

void UCI::cmd_quit() {
}

void UCI::cmd_debug(const std::string& args) {
    if (args == "on") {
        m_debug_mode = true;
    } else if (args == "off") {
        m_debug_mode = false;
    }
}

void UCI::cmd_setoption(const std::string& args) {
    std::istringstream iss(args);
    std::string token;
    std::string option_name;
    
    while (iss >> token) {
        if (token == "value") break;
        if (!option_name.empty()) option_name += " ";
        option_name += token;
    }
    
    if (token == "value" && iss >> token) {
        if (option_name == "Hash") {
            try {
                (void)std::stoi(token);
                m_searcher = std::make_unique<Searcher>();
            } catch (...) {
                // Invalid value, ignore
            }
        }
    }
}

void UCI::send_response(const std::string& msg) {
    std::cout << msg << std::endl;
    std::cout.flush();
}

void UCI::send_info(const std::string& msg) {
    if (m_debug_mode) {
        std::cout << "info " << msg << std::endl;
    }
}

void UCI::send_best_move(const std::string& move) {
    std::cout << "bestmove " << move << std::endl;
    std::cout.flush();
}

std::string UCI::move_to_uci(const Move& move) const {
    if (!move.is_valid()) {
        return "0000";
    }
    
    Square from = static_cast<Square>(move.from());
    Square to = static_cast<Square>(move.to());

    char from_file = 'a' + file_of(from);
    char from_rank = '1' + rank_of(from);
    char to_file = 'a' + file_of(to);
    char to_rank = '1' + rank_of(to);

    std::string uci;
    uci += from_file;
    uci += from_rank;
    uci += to_file;
    uci += to_rank;

    if (move.is_promotion()) {
        PieceType pt = move.promotion_piece();
        if (pt == QUEEN) uci += 'q';
        else if (pt == ROOK) uci += 'r';
        else if (pt == BISHOP) uci += 'b';
        else if (pt == KNIGHT) uci += 'n';
    }

    return uci;
}

bool UCI::parse_move(const std::string& str, Move& move) const {
    if (str.length() < 4 || str.length() > 5) return false;

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

    Color us = m_board.side_to_move();
    bool found_piece = false;
    PieceType pt_on_from = NONE;
    for (int p = PAWN; p <= KING; ++p) {
        if (m_board.pieces(static_cast<PieceType>(p), us) & (1ULL << from)) {
            found_piece = true;
            pt_on_from = static_cast<PieceType>(p);
            break;
        }
    }
    if (!found_piece) {
        return false;
    }

    move.set_from(from);
    move.set_to(to);
    
    if (str.length() == 5) {
        char prom = str[4];
        PieceType promo_piece = NONE;
        if (prom == 'q') promo_piece = QUEEN;
        else if (prom == 'r') promo_piece = ROOK;
        else if (prom == 'b') promo_piece = BISHOP;
        else if (prom == 'n') promo_piece = KNIGHT;
        else return false;
        
        bool is_cap = (m_board.all_pieces(static_cast<Color>(1 - us)) & (1ULL << to)) != 0;
        move.set_promotion(promo_piece, is_cap);
        return true;
    }
    
    if (from == SQ_E1 && to == SQ_G1 && us == WHITE) {
        move.set_type(CASTLE_KS);
        return true;
    }
    if (from == SQ_E1 && to == SQ_C1 && us == WHITE) {
        move.set_type(CASTLE_QS);
        return true;
    }
    if (from == SQ_E8 && to == SQ_G8 && us == BLACK) {
        move.set_type(CASTLE_KS);
        return true;
    }
    if (from == SQ_E8 && to == SQ_C8 && us == BLACK) {
        move.set_type(CASTLE_QS);
        return true;
    }
    
    if (pt_on_from == PAWN) {
        const int delta = static_cast<int>(to) - static_cast<int>(from);
        if (delta == 16 || delta == -16) {
            move.set_type(DOUBLE_PUSH);
            return true;
        }
    }
    
    if (m_board.en_passant() == to && pt_on_from == PAWN) {
        move.set_type(EN_PASSANT);
        return true;
    }
    
    if (m_board.all_pieces(static_cast<Color>(1 - us)) & (1ULL << to)) {
        move.set_type(CAPTURE);
    } else {
        move.set_type(QUIET);
    }

    return true;
}

bool UCI::parse_and_validate_move(const std::string& str, Move& move) const {
    if (!parse_move(str, move)) {
        return false;
    }

    return MoveValidator::is_legal(m_board, move);
}
