#include <iostream>
#include <fstream>
#include <sstream>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>
#include <bmlc/parser/ast.hpp>
#include <bmlc/parser/parser.hpp>

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
    bmlc::Parser parser(lexer, filename);

    auto program = parser.parse_program();

    return 0;
}