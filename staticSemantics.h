#ifndef STATSEM_H
#define STATSEM_H

#include <string>
#include <map>
#include "node.h"
#include "scanner.h"
#include "parser.h"

class STATSEM {
    public:
        struct VarInfo {
            int lineDeclared;
            bool initialized;
            int initValue;
        };
    private:
        std::map<std::string, VarInfo> varTable;

    public:
        void insert(const std::string& varName, int lineNumber, int initValue);
        bool verify(const std::string& varName);
        void checkVars();

        // getter for allocateStorage / other code
        const std::map<std::string, VarInfo>& getVarTable() const;
};
 
STATSEM staticSemantics(Node* root);

#endif // STATSEM_H