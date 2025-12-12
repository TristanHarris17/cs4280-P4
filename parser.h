#ifndef PARSER_H
#define PARSER_H

#include "scanner.h"
#include "token.h"
#include "node.h"

// global token used by the parser (defined in parser.cpp)
extern Token tk;

// parser entry point
Node* parser();

// grammar parsing functions
Node* program();
Node* vars();
Node* varList();
Node* block();
Node* stats();
Node* mStat();
Node* stat();

Node* read();
Node* print();
Node* cond();
Node* loop();
Node* assign();

Node* relational();

Node* exp();
Node* M();
Node* N();
Node* R();

void testTree(Node* root, int depth = 0);

#endif // PARSER_H