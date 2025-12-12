#ifndef STATSEM_H
#define STATSEM_H

#include <string>
#include <map>
#include "node.h"
#include "scanner.h"
#include "parser.h"

class STATSEM {
    private:
        struct VarInfo {
            int lineDeclared;
            bool initialized;
            int initValue;
        };
        std::map<std::string, VarInfo> varTable;

        public:
            void insert(const std::string& varName, int lineNumber, int initValue);
            bool verify(const std::string& varName);
            void checkVars();
};

STATSEM staticSemantics(Node* root);

#endif // STATSEM_H