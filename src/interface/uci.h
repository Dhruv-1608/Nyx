#ifndef UCI_H
#define UCI_H

#include "search.h"
#include "board.h"
#include "types.h"
#include <string>
#include <memory>
#include <map>

class UCI {
public:
    UCI();
    ~UCI();
    void run();

private:
    void cmd_uci();
    void cmd_isready();
    void cmd_position(const std::string& args);
    void cmd_go(const std::string& args);
    void cmd_stop();
    void cmd_quit();
    void cmd_debug(const std::string& args);
    void cmd_setoption(const std::string& args);
    void send_response(const std::string& msg);
    void send_info(const std::string& msg);
    void send_best_move(const std::string& move);
    std::string move_to_uci(const Move& move) const;
    bool parse_move(const std::string& str, Move& move) const;
    bool parse_and_validate_move(const std::string& str, Move& move) const;
    Board m_board;
    std::unique_ptr<Searcher> m_searcher;
    bool m_debug_mode;
    bool m_should_stop;
    struct Option {
        std::string name;
        std::string value;
    };
    std::map<std::string, Option> m_options;
};

#endif
