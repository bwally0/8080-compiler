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

    int start_line = line_;
    int start_column = column_;

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

    // keywords and identifiers
    if (std::isalpha(c) || c == '_') {
        std::string lexeme(1, c);
        while (std::isalnum(peek()) || peek() == '_') {
            lexeme.push_back(advance());
        }

        TokenType type = keyword_to_token_type(lexeme);
        return Token{type, lexeme, start_line, start_column};
    }

    // number literals
    if (std::isdigit(c)) {
        std::string lexeme(1, c);
        while (std::isdigit(peek())) {
            lexeme.push_back(advance());
        }
        return Token{TokenType::NUMBER, lexeme, start_line, start_column};
    }

    switch (c) {
        case '=': return Token{TokenType::ASSIGN, "=", start_line, start_column};
        case '+': return Token{TokenType::PLUS, "+", start_line, start_column};
        case '-': 
            if (peek() == '>') {
                advance(); // consume '>'
                return Token{TokenType::ARROW, "->", start_line, start_column};
            } else {
                return Token{TokenType::MINUS, "-", start_line, start_column};
            }
        case '<': return Token{TokenType::LESS, "<", start_line, start_column};
        case '&': return Token{TokenType::AND, "&", start_line, start_column};
        case '|': return Token{TokenType::OR, "|", start_line, start_column};
        case '^': return Token{TokenType::XOR, "^", start_line, start_column};
        case '(': return Token{TokenType::L_PAREN, "(", start_line, start_column};
        case ')': return Token{TokenType::R_PAREN, ")", start_line, start_column};
        case '{': return Token{TokenType::L_BRACE, "{", start_line, start_column};
        case '}': return Token{TokenType::R_BRACE, "}", start_line, start_column};
        case ',': return Token{TokenType::COMMA, ",", start_line, start_column};
        case ';': return Token{TokenType::SEMICOLON, ";", start_line, start_column};
    }

    throw std::runtime_error(filename_ + ":" + std::to_string(start_line) + ":" + std::to_string(start_column) +
                             ": error: unexpected character '" + c + "'");
}

}