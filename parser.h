#pragma once

#include <string>
#include <regex>

class Parser {
    std::string input;
    unsigned long pos;

public:

    struct State {
        unsigned long saved_pos;
        Parser* p;

        State(Parser* p);
        bool rollback();
        bool commit();
        bool commit(int start, int end);
        bool commit(const std::string& result);
    };

    std::smatch last_re_match;
    std::string last_match;

    Parser(std::string input);

    State save_point();

    bool is_eof();
    void skip_space();
    void skip_rest_of_line(bool continuable);

    bool match(char c);
    bool match(const std::string& str);
    bool match_re(const std::string& r, bool space = true);
    bool match_any();

    bool match_comment();

    bool match_block_comment();
    bool match_line_comment();

    bool match_macro();
    bool match_quoted(const char *left, const char *right);
    bool match_code();

    bool match_number();
    bool match_identifier();

    bool peek(const char c);
    bool peek(const std::string& str);
    bool peek_re(const std::string& r, bool space = true);

    char current() const;
};
