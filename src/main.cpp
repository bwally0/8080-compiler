#include <iostream>
#include <fstream>
#include <sstream>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>

int main() {
    std::string filename("fib.bml");
    std::string filepath("./tests/fib.bml");
    std::ifstream file(filepath);   
    if (!file.is_open()) {
        std::cerr << "error: failed to open source file: '" << filename << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    bmlc::Lexer lexer(source, filename);

    bmlc::Token token;
    do {
        try {
            token = lexer.next_token();
            std::cout << token << std::endl;
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
        }

    } while (token.type != bmlc::TokenType::END_OF_FILE);

    return 0;
}