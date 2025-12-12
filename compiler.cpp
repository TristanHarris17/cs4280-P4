#include <iostream>
#include <fstream>
#include <cctype>
#include "node.h"
#include "token.h"
#include "compiler.h"

// create temporary variable names
static int tempVarCounter = 0;
std::string createTempVar() {
    return "t" + std::to_string(tempVarCounter++);
}

// create labels for branching
static int labelCounter = 0;
std::string createLabel() {
    return "L" + std::to_string(labelCounter++);
}

// allocate storage for variables after code generation
void allocateStorage(STATSEM& statsem, std::ofstream& out) {
    const auto& table = statsem.getVarTable(); // access varTable
    for (const auto& entry : table) { // entry: pair<const string, VarInfo>
        const std::string& name = entry.first;
        const STATSEM::VarInfo& info = entry.second;
        out << name << " " << info.initValue << "\n"; // allocate with initial value
    }

    // allocate temp variables
    for (int i = 0; i < tempVarCounter; ++i) {
        out << "t" << i << " 0\n"; // initialize temps to 0
    }
}

// traversal implementation
static void traversal_impl(Node* root, std::ofstream& out) {
    if (!root) return;
    std::cout << "Visiting " << root->type << std::endl;
    // if else to generate code based on node type
    if (root->type == "read") {
        // read input into variable
        std::string varName = root->tokens[0];
        out << "READ " << varName << "\n";
    }
    else if (root->type == "print") {
        // continue to child to get expression value
        traversal_impl(root->children[0], out);
        // store expression result in temp variable
        std::string tempVar = createTempVar();
        out << "STORE " << tempVar << "\n";
        // print the result
        out << "WRITE " << tempVar << "\n";
    }
    else if (root->type == "cond") {
        // cond [ identifier <relational> <exp> ] <stat>
        if (root->tokens.empty() || root->children.size() < 3) return;

        // evaluate RHS <exp> -> leave result in ACC
        traversal_impl(root->children[1], out);
        // save RHS
        std::string rhsTemp = createTempVar();
        out << "STORE " << rhsTemp << "\n";

        // load identifier (LHS) and compute LHS - RHS in ACC
        std::string id = root->tokens[1];
        out << "LOAD " << id << "\n";
        out << "SUB " << rhsTemp << "\n";

        // relational operator is stored as a token in children[0]
        std::string relTok;
        if (!root->children[0]->tokens.empty()) relTok = root->children[0]->tokens[0];

        std::string trueLabel = createLabel();
        std::string endLabel  = createLabel();

        if (relTok == ";") {
            // NOT EQUAL: if ACC == 0 skip stat, else fall through to stat
            out << "BRZERO " << endLabel << "\n";
            traversal_impl(root->children[2], out);
            out << endLabel << ": NOOP\n";
        } else {
            std::string instr;
            if (relTok == "?le") instr = "BRZNEG";   // ACC <= 0 -> true
            else if (relTok == "?lt") instr = "BRNEG";   // ACC < 0 -> true
            else if (relTok == "?ge") instr = "BRZPOS";  // ACC >= 0 -> true
            else if (relTok == "?eq" || relTok == "==" || relTok == "= =") instr = "BRZERO"; // ACC == 0 -> true
            else instr = "BRZERO"; // conservative default (equality)

            // branch-to-true pattern: if true -> jump to trueLabel, else jump past true block
            out << instr << " " << trueLabel << "\n";
            out << "BR " << endLabel << "\n";
            out << trueLabel << ": NOOP\n";
            traversal_impl(root->children[2], out);
            out << endLabel << ": NOOP\n";
        }
    }
    else if (root->type == "loop") {
        // loop [ identifier <relational> <exp> ] <stat>
        if (root->tokens.empty() || root->children.size() < 3) return;

        std::string startLabel = createLabel();
        std::string bodyLabel  = createLabel();
        std::string endLabel   = createLabel();

        out << startLabel << ": NOOP\n";

        // evaluate RHS <exp> -> leave result in ACC
        traversal_impl(root->children[1], out);
        // save RHS
        std::string rhsTemp = createTempVar();
        out << "STORE " << rhsTemp << "\n";

        // load identifier (LHS) and compute LHS - RHS in ACC
        std::string id = root->tokens[1];
        out << "LOAD " << id << "\n";
        out << "SUB " << rhsTemp << "\n";

        // relational operator token in children[0]
        std::string relTok;
        if (!root->children[0]->tokens.empty()) relTok = root->children[0]->tokens[0];

        if (relTok == ";") {
            // NOT EQUAL: if ACC == 0 -> exit loop, else fall through to body
            out << "BRZERO " << endLabel << "\n";
            traversal_impl(root->children[2], out);
            out << "BR " << startLabel << "\n";
            out << endLabel << ": NOOP\n";
        } else {
            std::string instr;
            if (relTok == "?le") instr = "BRZNEG";   // ACC <= 0 -> enter body
            else if (relTok == "?lt") instr = "BRNEG";   // ACC < 0 -> enter body
            else if (relTok == "?ge") instr = "BRZPOS";  // ACC >= 0 -> enter body
            else if (relTok == "?eq" || relTok == "==" || relTok == "= =") instr = "BRZERO"; // ACC == 0 -> enter body
            else instr = "BRZERO";

            out << instr << " " << bodyLabel << "\n";
            out << "BR " << endLabel << "\n";
            out << bodyLabel << ": NOOP\n";
            traversal_impl(root->children[2], out);
            out << "BR " << startLabel << "\n";
            out << endLabel << ": NOOP\n";
        }
    }
    else if (root->type == "assign") {
        // set identifier = <exp> :
        std::string varName = root->tokens[0];
        // evaluate expression -> leave result in ACC
        traversal_impl(root->children[0], out);
        // store result into variable
        out << "STORE " << varName << "\n";
    }
    else if (root->type == "exp") {
        if (!root->tokens.empty() && root->tokens[0] == "**") {
            // multiplication
            // call right child
            traversal_impl(root->children[1], out);
            // store right child result in temp variable
            std::string tempVar = createTempVar(); 
            out << "STORE " << tempVar << "\n";
            // call left child
            traversal_impl(root->children[0], out);
            // multiply with right child result
            out << "MULT " << tempVar << "\n";
        } 
        else if (!root->tokens.empty() && root->tokens[0] == "//") {
            // integer division
            // call right child
            traversal_impl(root->children[1], out);
            // store right child result in temp variable
            std::string tempVar = createTempVar(); 
            out << "STORE " << tempVar << "\n";
            // call left child
            traversal_impl(root->children[0], out);
            // divide by right child result
            out << "DIV " << tempVar << "\n";
        } 
        else {
            // single M child
            traversal_impl(root->children[0], out);
        }
    } 
    else if (root->type == "M") {
        if (!root->tokens.empty() && root->tokens[0] == "+") {
            // addition
            // call right child
            traversal_impl(root->children[1], out);
            // store right child result in temp variable
            std::string tempVar = createTempVar(); 
            out << "STORE " << tempVar << "\n";
            // call left child
            traversal_impl(root->children[0], out);
            // add right child result
            out << "ADD " << tempVar << "\n";
        } 
        else {
            // single N child
            traversal_impl(root->children[0], out);
        }
    } 
    else if (root->type == "N") {
        // <N> -> <R> - <N> | - <N> | <R>
        if (!root->tokens.empty() && root->tokens[0] == "-") {
            if (root->children.size() == 1) {
                // unary minus: - <N>
                traversal_impl(root->children[0], out);
                std::string tempVar = createTempVar();
                out << "STORE " << tempVar << "\n";
                out << "LOAD 0\n";
                out << "SUB " << tempVar << "\n";
            } else if (root->children.size() >= 2) {
                // binary subtraction
                // evaluate right child
                traversal_impl(root->children[1], out);
                // store right child result in temp variable
                std::string tempVar = createTempVar();
                out << "STORE " << tempVar << "\n";
                // evaluate left child
                traversal_impl(root->children[0], out);
                // subtract right child result
                out << "SUB " << tempVar << "\n";
            }
        } else {
            // single <R> child
            if (!root->children.empty()) traversal_impl(root->children[0], out);
        }
    } 
    else if (root->type == "R") {
        if (root->children.size() == 1 && root->children[0]->type == "exp") {
            // case: exp
            traversal_impl(root->children[0], out);
        }
        // TODO: may not need both cases becuase you print LOAD either way
        else if (std::isalpha(static_cast<unsigned char>(root->tokens[0][0]))) {
            // case: identifier
            out << "LOAD " << root->tokens[0] << "\n";
        }
        else {
            // case: integer
            out << "LOAD " << root->tokens[0] << "\n";
        }
    }
    else {
        for (auto child : root->children) {
            traversal_impl(child, out);
        }
    }
}

// main traversal function
void traversal(Node* root, std::ofstream& out, STATSEM& statsem) {
    traversal_impl(root, out);
    out << "STOP" << std::endl;
    allocateStorage(statsem, out);
}