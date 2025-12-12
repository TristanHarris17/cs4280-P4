#include <iostream>
#include <cstdlib>
#include <cctype>
#include <functional>

#include "staticSemantics.h"

void STATSEM::insert(const std::string& varName, int lineNumber, int initValue) {
    if (varTable.find(varName) != varTable.end()) {
        std::cerr << "ERROR in P3 on line " << lineNumber << ": Variable '" << varName << "' already declared on line " << varTable[varName].lineDeclared << ".\n";
        exit(EXIT_FAILURE);
    }
    varTable[varName] = {lineNumber, false, initValue};
}
    
bool STATSEM::verify(const std::string& varName) {
    auto it = varTable.find(varName);
    if (it == varTable.end()) {
        return false;
    }
    it->second.initialized = true;
    return true;
}

void STATSEM::checkVars() {
    for (const auto& entry : varTable) {
        if (!entry.second.initialized) {
            std::cerr << "WARNING in P3: Variable '" << entry.first << "' declared on line " << entry.second.lineDeclared << " but never used.\n";
        }
    }

    
    // Print symbol table for testing
    std::cout << "Symbol table (variable -> declared line, initialized):\n";
    for (const auto& entry : varTable) {
        std::cout << "  " << entry.first << " -> " << entry.second.lineDeclared
                  << ", " << (entry.second.initialized ? "true" : "false") << "\n"
                    << "    Initial Value: " << entry.second.initValue << "\n";
    }

}

STATSEM staticSemantics(Node* root) {
    STATSEM statsem;
    if (!root) {
        statsem.checkVars();
        return statsem;
    }

    auto isIdentifier = [](const std::string &s) -> bool {
        if (s.empty()) return false;
        unsigned char c = static_cast<unsigned char>(s[0]);
        if (!std::isalpha(c) && s[0] != '_') return false;
        // exclude reserved keywords used in the grammar
        static const std::vector<std::string> keywords = {
            "go", "int", "exit", "scan", "output", "cond", "loop", "set"
        };
        for (const auto &kw : keywords) if (s == kw) return false;
        return true;
    };

    std::function<void(Node*)> traverse = [&](Node* node) {
        if (!node) return;

        // Preorder handling
        if (node->type == "vars") {
            // tokens: ["int", identifier, "=", number, ... "]
                const std::string &tok = node->tokens[0];
                const std::string initValue = node->tokens[1];
                if (isIdentifier(tok)) {
                    statsem.insert(tok, node->line_numbers[0], std::stoi(initValue));
                }
        } else if (node->type == "varList") {
            // tokens: [identifier, "=", number]
            if (!node->tokens.empty() && !node->line_numbers.empty()) {
                const std::string &tok = node->tokens[0];
                const std::string initValue = node->tokens[1];
                if (isIdentifier(tok)) statsem.insert(tok, node->line_numbers[0], std::stoi(initValue));
            }
        } else {
            // For all other nodes, verify any identifier tokens used
            for (size_t i = 0; i < node->tokens.size() && i < node->line_numbers.size(); ++i) {
                const std::string &tok = node->tokens[i];
                if (isIdentifier(tok)) {
                    bool isValid = statsem.verify(tok);
                    if (!isValid) { // handle undeclared variable
                        std::cerr << "ERROR in P3 on line " << node->line_numbers[i] << ": Variable '" << tok << "' used before declaration.\n";
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        // Recurse children in order
        for (Node* ch : node->children) traverse(ch);
    };

    traverse(root);
    statsem.checkVars();
    return statsem;
}