#include <iostream>
#include <fstream>
#include "node.h"
#include "token.h"
#include "compiler.h"

static void traversal_impl(Node* root, std::ofstream& out) {
    if (!root) return;
    std::cout << "Visiting " << root->type << std::endl;
    for (auto child : root->children) {
        traversal_impl(child, out);
    }
}

void traversal(Node* root, std::ofstream& out) {
    traversal_impl(root, out);
    out << "STOP" << std::endl;
}