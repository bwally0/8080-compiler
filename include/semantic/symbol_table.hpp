#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <bmlc/parser/ast.hpp>

namespace bmlc {

struct Symbol {
    std::string name;
    SourceLocation location;
    Type type;
    bool is_function;
    std::vector<Param> parameters;
};

class SymbolTable {
public:
    SymbolTable();

    void define(const Symbol& symbol);
    Symbol* lookup(const std::string& name);
    bool is_defined(const std::string& name) const;

private:
    std::unordered_map<std::string, Symbol> symbols_;
};

} // namespace bmlc