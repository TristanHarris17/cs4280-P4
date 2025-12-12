#include <iostream>
#include <fstream>
#include "node.h"
#include "token.h"
#include "compiler.h"

void allocateStorage(STATSEM& statsem, std::ofstream& out) {
    const auto& table = statsem.getVarTable();
    for (const auto& entry : table) {
        const std::string& name = entry.first;
        const STATSEM::VarInfo& info = entry.second;
        out << name << " " << info.initValue << "\n";
    }
}

static void traversal_impl(Node* root, std::ofstream& out) {
    if (!root) return;
    std::cout << "Visiting " << root->type << std::endl;
    for (auto child : root->children) {
        traversal_impl(child, out);
    }
}

void traversal(Node* root, std::ofstream& out, STATSEM& statsem) {
    traversal_impl(root, out);
    out << "STOP" << std::endl;
    allocateStorage(statsem, out);
}