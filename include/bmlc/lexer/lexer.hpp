#pragma once

#include <string>
#include <bmlc/lexer/token.hpp>

namespace bmlc {

// Lexer produces stream of tokens given source code input,
// this stream is later consumed by the parser.
class Lexer {
public:
    explicit Lexer(const std::string &source, const std::string &filename);

    Token next_token();

private:
    std::string source_;
    std::string filename_;
    size_t current_pos_;
    int line_;
    int column_;

    char peek() const;
    char peek_next() const;
    char advance();
    bool is_at_end() const;
    void skip_whitespace();
};

} // namespace bmlc