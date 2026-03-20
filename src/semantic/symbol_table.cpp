#include <bmlc/semantic/symbol_table.hpp>

namespace bmlc {

SymbolTable::SymbolTable() = default;

void SymbolTable::define(const Symbol& symbol) {
    symbols_[symbol.name] = symbol;
}

Symbol* SymbolTable::lookup(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

bool SymbolTable::is_defined(const std::string& name) const {
    return symbols_.find(name) != symbols_.end();
}

void SymbolTable::mark_as_used(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        it->second.is_used = true;
    }
}

std::vector<Symbol*> SymbolTable::get_unused_symbols() const {
    std::vector<Symbol*> unused;
    for (auto& pair : symbols_) {
        if (!pair.second.is_used && pair.first != "main") {
            unused.push_back(const_cast<Symbol*>(&pair.second));
        }
    }
    return unused;
}

bool SymbolTable::has_main() const {
    auto it = symbols_.find("main");
    return it != symbols_.end() && it->second.is_function;
}

const std::vector<Symbol*> SymbolTable::get_all_symbols() const {
    std::vector<Symbol*> all_symbols;
    for (auto& pair : symbols_) {
        all_symbols.push_back(const_cast<Symbol*>(&pair.second));
    }
    return all_symbols;
}

} // namespace bmlc
