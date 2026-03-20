#include <iostream>
#include <fstream>
#include <sstream>

#include <bmlc/lexer/token.hpp>
#include <bmlc/lexer/lexer.hpp>
#include <bmlc/parser/ast.hpp>
#include <bmlc/parser/parser.hpp>
#include <bmlc/semantic/semantic_analyzer.hpp>

int main(int argc, char* argv[]) {
    std::string filepath = (argc > 1) ? argv[1] : "./tests/fib.bml";
    std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    
    std::ifstream file(filepath);   
    if (!file.is_open()) {
        std::cerr << "error: failed to open source file: '" << filepath << "'" << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    bmlc::Lexer lexer(source, filename);
    
    bmlc::Parser parser(lexer, filename);
    auto program = parser.parse_program();
    
    bmlc::SemanticAnalyzer analyzer;
    analyzer.analyze(*program);
    
    //print_ast(*program);
    
    // Report warnings
    if (analyzer.has_warnings()) {
        for (const auto& warning : analyzer.get_warnings()) {
            std::cerr << warning << std::endl;
        }
    }
    
    // Report errors
    if (analyzer.has_errors()) {
        for (const auto& error : analyzer.get_errors()) {
            std::cerr << error << std::endl;
        }
        return 1;
    }

    return 0;
}