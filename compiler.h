#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <fstream>
#include "node.h"
#include "token.h"
#include "staticSemantics.h"

void traversal(Node* root, std::ofstream& out, STATSEM& statsem);

#endif 