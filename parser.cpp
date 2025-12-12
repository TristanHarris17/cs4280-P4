#include <iostream>

#include "scanner.h"
#include "token.h"
#include "parser.h"
#include "node.h"

/*
BNF Grammar:
<program>  ->     go <vars> <block> exit
<vars>         ->      empty | int identifier = integer <varList> :
<varList>     ->      identifier = integer <varList> | empty
<block>       ->      { <vars> <stats> }
<stats>         ->      <stat> <mStat>
<mStat>       ->      empty |  <stat> <mStat>
<stat>           ->      <read>   | <print>   | <block> | <cond>  | <loop>  | <assign>
<read>         ->      scan identifier :
<print>        ->     output <exp> :
<cond>        ->     cond [ identifier <relational> <exp> ] <stat>
<loop>         ->     loop [ identifier <relational> <exp> ]  <stat>
<assign>      ->     set identifier = <exp> :
<relational> ->      ?le  | ?ge | ?lt | ?eq | ?ne | ?gt | ; | = =    added ?ne and ?gt to match scanner lexical definitions from P1                  
<exp>          ->      <M> ** <exp> | <M> // <exp> | <M>
<M>             ->      <N> + <M> | <N>      
<N>             ->      <R> - <N> | - <N> |  <R>
<R>              ->      ( <exp> )  | identifier | integer   
*/

// globals
Token tk;

Node* parser() {
    tk = scanner();
    Node* root = program();
    if (tk.group != TokenGroup::END_OF_FILE) {
        std::cerr << "Syntax Error: Extra tokens after program end at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* program() {
    Node* root = new Node();
    // do not store the "go" keyword as a token; node->type already identifies the node
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "go") {
        root->type = "program";
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected 'go' at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(vars());
    root->children.push_back(block());
    // do not store the "exit" keyword as a token
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "exit") {
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected 'exit' at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* vars() {
    Node* root = new Node();
    root->type = "vars";
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "int") {
        // consume 'int' keyword but do not store it in tokens
        tk = scanner();
        if (tk.group == TokenGroup::IDENTIFIER) {
            // store only the identifier name and its line number
            root->tokens.push_back(tk.instance);
            root->line_numbers.push_back(tk.line);
            tk = scanner();
            if (tk.group == TokenGroup::OPERATOR && tk.instance == "=") {
                tk = scanner();
                if (tk.group == TokenGroup::NUMBER) {
                    // optional: store initial value (keeps alignment)
                    root->tokens.push_back(tk.instance);
                    root->line_numbers.push_back(tk.line);
                    tk = scanner();
                    // continue with varList (varList will add more identifier/value pairs)
                    Node* vl = varList();
                    if (vl) root->children.push_back(vl);
                    if (tk.group == TokenGroup::DELIMITER && tk.instance == ":") {
                        tk = scanner();
                    } else {
                        std::cerr << "Syntax Error: Expected ':' after variable declaration at line " << tk.line << std::endl;
                        exit(EXIT_FAILURE);
                    }
                } else {
                    std::cerr << "Syntax Error: Expected integer literal in variable declaration at line " << tk.line << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                std::cerr << "Syntax Error: Expected '=' after identifier in variable declaration at line " << tk.line << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Syntax Error: Expected identifier after 'int' at line " << tk.line << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        return nullptr; // empty production
    }
    return root;
}

Node* varList() {
    Node* root = new Node();
    root->type = "varList";
    if (tk.group == TokenGroup::IDENTIFIER) {
        // identifier = integer <varList> | empty
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        if (tk.group == TokenGroup::OPERATOR && tk.instance == "=") {
            tk = scanner();
            if (tk.group == TokenGroup::NUMBER) {
                root->tokens.push_back(tk.instance); // store value as well (optional)
                root->line_numbers.push_back(tk.line);
                tk = scanner();
                // recursively handle more declarations
                Node* more = varList();
                if (more) root->children.push_back(more);
            } else {
                std::cerr << "Syntax Error: Expected integer in varList at line " << tk.line << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Syntax Error: Expected '=' in varList at line " << tk.line << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        return nullptr; // empty production
    }
    return root;
}

Node* block() {
    Node* root = new Node();
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "{") {
        root->type = "block";
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected '{' at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(vars());
    root->children.push_back(stats());
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "}") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected '}' at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* stats() {;
    Node* root = new Node();
    root->type = "stats";
    root->children.push_back(stat());
    root->children.push_back(mStat());
    return root;
}

Node* mStat() {
    Node* root = new Node();
    root->type = "mStat";
    if (tk.group == TokenGroup::KEYWORD || (tk.group == TokenGroup::DELIMITER && tk.instance == "{")) {
        root->children.push_back(stat());
        root->children.push_back(mStat());
    } else {
        return nullptr;
    }
    return root;
}

Node* stat() {
    Node* root = new Node();
    root->type = "stat";
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "scan") {
        root->children.push_back(read());
    } else if (tk.group == TokenGroup::KEYWORD && tk.instance == "output") {
        root->children.push_back(print());
    } else if (tk.group == TokenGroup::DELIMITER && tk.instance == "{") {
        root->children.push_back(block());
    } else if (tk.group == TokenGroup::KEYWORD && tk.instance == "cond") {
        root->children.push_back(cond());
    } else if (tk.group == TokenGroup::KEYWORD && tk.instance == "loop") {
        root->children.push_back(loop());
    } else if (tk.group == TokenGroup::KEYWORD && tk.instance == "set") {
        root->children.push_back(assign());
    } else {
        std::cerr << "Syntax Error: Invalid statement at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* read() {
    Node* root = new Node();
    root->type = "read";
    // consume 'scan' keyword but do not store it as a token
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "scan") {
        tk = scanner();
        if (tk.group == TokenGroup::IDENTIFIER) {
            // store identifier name and line (this is a use)
            root->tokens.push_back(tk.instance);
            root->line_numbers.push_back(tk.line);
            tk = scanner();
            if (tk.group == TokenGroup::DELIMITER && tk.instance == ":") {
                tk = scanner();
                return root;
            } else {
                std::cerr << "Syntax Error: Expected ':' after scan statement at line " << tk.line << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Syntax Error: Expected identifier after 'scan' at line " << tk.line << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        std::cerr << "Syntax Error: Expected 'scan' at line " << tk.line << std::endl;
        exit(EXIT_FAILURE);
    }
    return root;
}

Node* print() {
    Node* root = new Node();
    root->type = "print";
    tk = scanner();
    root->children.push_back(exp());
    if (tk.group == TokenGroup::DELIMITER && tk.instance == ":") {
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected ':' at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* cond() {
    Node* root = new Node();
    tk = scanner();
    root->type = "cond";
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "[") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected '[' at line " << tk.line << std::endl;
        exit(1);
    }
    if (tk.group == TokenGroup::IDENTIFIER) {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected identifier at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(relational());
    root->children.push_back(exp());
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "]") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected ']' at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(stat());
    return root;
}

Node* loop() {
    Node* root = new Node();
    tk = scanner();
    root->type = "loop";
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "[") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected '[' at line " << tk.line << std::endl;
        exit(1);
    }
    if (tk.group == TokenGroup::IDENTIFIER) {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected identifier at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(relational());
    root->children.push_back(exp());
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "]") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected ']' at line " << tk.line << std::endl;
        exit(1);
    }
    root->children.push_back(stat());
    return root;
}

Node* assign() {
    Node* root = new Node();
    root->type = "assign";
    // assume tk is 'set'
    if (tk.group == TokenGroup::KEYWORD && tk.instance == "set") {
        tk = scanner();
        if (tk.group == TokenGroup::IDENTIFIER) {
            // store the identifier being assigned (use for verification)
            root->tokens.push_back(tk.instance);
            root->line_numbers.push_back(tk.line);
            tk = scanner();
            if (tk.group == TokenGroup::OPERATOR && tk.instance == "=") {
                tk = scanner();
                // the expression will create its own nodes and tokens for identifiers/integers
                root->children.push_back(exp());
                if (tk.group == TokenGroup::DELIMITER && tk.instance == ":") {
                    tk = scanner();
                    return root;
                } else {
                    std::cerr << "Syntax Error: Expected ':' after assignment at line " << tk.line << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                std::cerr << "Syntax Error: Expected '=' in assignment at line " << tk.line << std::endl;
                exit(EXIT_FAILURE);
            }
        } else {
            std::cerr << "Syntax Error: Expected identifier after 'set' at line " << tk.line << std::endl;
            exit(EXIT_FAILURE);
        }
    } else {
        std::cerr << "Syntax Error: Expected 'set' at line " << tk.line << std::endl;
        exit(EXIT_FAILURE);
    }
    return root;
}

Node* relational() {
    Node* root = new Node();
    root->type = "relational";
    if (tk.group == TokenGroup::OPERATOR &&
        (tk.instance == "?le" || tk.instance == "?ge" || tk.instance == "?lt" ||
         tk.instance == "?eq" || tk.instance == "?ne" || tk.instance == "?gt" || tk.instance == ";" || tk.instance == "= =")) { // added ?ne and ?gt to match scanner lexical definitions from P1
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
    } else {
        std::cerr << "Syntax Error: Expected relational operator at line " << tk.line << std::endl;
        exit(1);
    }
    return root;
}

Node* exp() {
    Node* root = new Node();
    root->type = "exp";
    root->children.push_back(M());
    if (tk.group == TokenGroup::OPERATOR && (tk.instance == "**" || tk.instance == "//")) {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        root->children.push_back(exp());
    }
    return root;
}

Node* M() {
    Node* root = new Node();
    root->type = "M";
    root->children.push_back(N());
    if (tk.group == TokenGroup::OPERATOR && tk.instance == "+") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        root->children.push_back(M());
    }
    return root;
}

Node* N() {
    Node* root = new Node();
    root->type = "N";
    if (tk.group == TokenGroup::OPERATOR && tk.instance == "-") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        root->children.push_back(N());
    } else {
        root->children.push_back(R());
        if (tk.group == TokenGroup::OPERATOR && tk.instance == "-") {
            root->tokens.push_back(tk.instance);
            root->line_numbers.push_back(tk.line);
            tk = scanner();
            root->children.push_back(N());
        }
    }
    return root;
}

Node* R() {
    Node* root = new Node();
    root->type = "R";
    if (tk.group == TokenGroup::DELIMITER && tk.instance == "(") {
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        Node* e = exp();
        root->children.push_back(e);
        if (tk.group == TokenGroup::DELIMITER && tk.instance == ")") {
            root->tokens.push_back(tk.instance);
            root->line_numbers.push_back(tk.line);
            tk = scanner();
            return root;
        } else {
            std::cerr << "Syntax Error: Expected ')' at line " << tk.line << std::endl;
            exit(EXIT_FAILURE);
        }
    } else if (tk.group == TokenGroup::IDENTIFIER) {
        // store identifier use
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        return root;
    } else if (tk.group == TokenGroup::NUMBER) {
        // store number literal (optional for semantics)
        root->tokens.push_back(tk.instance);
        root->line_numbers.push_back(tk.line);
        tk = scanner();
        return root;
    } else {
        std::cerr << "Syntax Error: Expected identifier, integer, or '(' at line " << tk.line << std::endl;
        exit(EXIT_FAILURE);
    }
    return root;
}
    
// preorder printer for the parse tree with indentation
void testTree(Node* node, int depth) {
    if (!node) return;
    std::string indent(depth * 2, ' ');
    // Print node type with indentation
    std::cout << indent << node->type;
    // Print tokens if present
    if (!node->tokens.empty()) {
        std::cout << " |";
        for (const auto &t : node->tokens) std::cout << " " << t;
    }
    // Print line numbers if present
    if (!node->line_numbers.empty()) {
        std::cout << " |";
        for (const auto &ln : node->line_numbers) std::cout << " " << ln;
    }
    std::cout << std::endl;
    // Recurse children in order with increased depth
    for (auto *ch : node->children) testTree(ch, depth + 1);
}

