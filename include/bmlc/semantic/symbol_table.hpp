#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <set>
#include <bmlc/parser/ast.hpp>

namespace bmlc {

struct Symbol {
    std::string name;
    SourceLocation location;
    Type type;
    bool is_function;
    std::vector<Param> parameters;
    mutable bool is_used = false;
};

class SymbolTable {
public:
    SymbolTable();

    void define(const Symbol& symbol);
    Symbol* lookup(const std::string& name);
    bool is_defined(const std::string& name) const;
    
    void mark_as_used(const std::string& name);
    std::vector<Symbol*> get_unused_symbols() const;
    
    bool has_main() const;
    const std::vector<Symbol*> get_all_symbols() const;

private:
    std::unordered_map<std::string, Symbol> symbols_;
};

} // namespace bmlc