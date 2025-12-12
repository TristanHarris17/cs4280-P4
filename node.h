#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

typedef struct Node {
    std::string type;
    std::vector<std::string> tokens;
    std::vector<int> line_numbers;
    std::vector<Node*> children;
} Node;

#endif