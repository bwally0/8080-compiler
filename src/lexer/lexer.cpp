#include <string>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>

namespace bmlc {

// constructor, pass source code as string.
Lexer::Lexer(const std::string &source, const std::string &filename)
    : source_(source), filename_(filename), current_pos_(0), line_(1), column_(1) {}

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
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

Token Lexer::next_token() {
    skip_whitespace();

    if (is_at_end()) {
        return Token{TokenType::END_OF_FILE, "", line_, column_};
    }

    char c = advance();

    // skip comments
    if (c == '/' && peek() == '*') {
        advance(); // consume '*'
        while (!is_at_end() && !(peek() == '*' && peek_next() == '/')) {
            advance();
        }
        if (!is_at_end()) {
            advance(); // consume '*'
            advance(); // consume '/'
        }
        return next_token(); // get next token
    }

    switch (c) {
        case '=': return Token{TokenType::ASSIGN, "=", line_, column_};
        case '+': return Token{TokenType::PLUS, "+", line_, column_};
        case '-': 
            if (peek() == '>') {
                advance(); // consume '>'
                return Token{TokenType::ARROW, "->", line_, column_};
            } else {
                return Token{TokenType::MINUS, "-", line_, column_};
            }
        case '<': return Token{TokenType::LESS, "<", line_, column_};
        case '&': return Token{TokenType::AND, "&", line_, column_};
        case '|': return Token{TokenType::OR, "|", line_, column_};
        case '^': return Token{TokenType::XOR, "^", line_, column_};
        case '(': return Token{TokenType::L_PAREN, "(", line_, column_};
        case ')': return Token{TokenType::R_PAREN, ")", line_, column_};
        case '{': return Token{TokenType::L_BRACE, "{", line_, column_};
        case '}': return Token{TokenType::R_BRACE, "}", line_, column_};
        case ',': return Token{TokenType::COMMA, ",", line_, column_};
        case ';': return Token{TokenType::SEMICOLON, ";", line_, column_};
    }

    throw std::runtime_error(filename_ + ":" + std::to_string(line_) + ":" + std::to_string(column_) +
                             ": error: unexpected character '" + c + "'");
}

}