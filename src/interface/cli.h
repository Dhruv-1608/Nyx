#ifndef CLI_H
#define CLI_H

#include <string>
#include <memory>

class Board;
class Searcher;

class CLI {
public:
    CLI();
    ~CLI();
    void run();

private:
    std::unique_ptr<Board> m_board;
    std::unique_ptr<Searcher> m_searcher;
    void print_board() const;
    void print_move_list() const;
    void make_human_move();
    void make_engine_move(int depth);
    bool parse_command(const std::string& cmd);
    std::string move_to_string(const Move& move) const;
    bool parse_move(const std::string& str, Move& move) const;
};

#endif
