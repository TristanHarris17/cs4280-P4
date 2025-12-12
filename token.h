#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenGroup {
    KEYWORD,
    NUMBER,
    OPERATOR,
    DELIMITER,
    IDENTIFIER,
    END_OF_FILE,
};

struct Token {
    TokenGroup group;
    std::string instance;
    int line;
};

inline const char* tokenGroupName(TokenGroup g) {
    switch (g) {
        case TokenGroup::KEYWORD: return "KEYWORD";
        case TokenGroup::NUMBER: return "NUMBER";
        case TokenGroup::OPERATOR: return "OPERATOR";
        case TokenGroup::DELIMITER: return "DELIMITER";
        case TokenGroup::IDENTIFIER: return "IDENTIFIER";
        case TokenGroup::END_OF_FILE: return "EOF";
    }
    return "Unknown";
}

#endif