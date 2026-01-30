#include <string>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>

namespace bmlc {

// constructor, pass source code as string.
Lexer::Lexer(const std::string &source)
    : source_(source), current_pos_(0), line_(1), column_(1) {}

// look at current character without consuming.
char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_pos_];
}

// look at next character without consuming.
char Lexer::peek_next() const {
    if (current_pos_ + 1 >= source_.length()) return '\0';
    return source_[current_pos_ + 1];
}

// consume current character and return it.
char Lexer::advance() {
    char c = peek();
    current_pos_++;

    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }

    return c;
}

// check if at end of source.
bool Lexer::is_at_end() const {
    return current_pos_ >= source_.length();
}

void Lexer::skip_whitespace() {

}

Token Lexer::next_token() {

}

}