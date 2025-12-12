#include "parser.h"
#include "staticSemantics.h"

#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    if (argc == 2) { // filename provided
        std::string filename = argv[1];
        filename += ".fs25s1";
        std::ifstream in(filename);
        if (!in) {
            std::cerr << "Could not open file: " << filename << std::endl;
            std::exit(1);
        }
        initScanner(in);
        Node* root = parser();
        staticSemantics(root);
    } else if (argc == 1) { // no filename read from stdin
        std::cout << "Taking keyboard input" << std::endl;
        initScanner(std::cin);
        Node* root = parser();
        staticSemantics(root);
    } else {
        std::cerr << "Usage: " << argv[0] << " <name>" << std::endl;
        return 1;
    }
    return 0;
}