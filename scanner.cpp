#include "scanner.h"

#include <fstream>
#include <iostream>
#include <cctype>
#include <unordered_set>
#include <vector>

static std::vector<Token> tokens;

static const std::unordered_set<std::string> keywords = {
    "go","og","loop","int","exit","scan","output","cond","then","set","func","program"
};

static const std::unordered_set<std::string> multi_ops = {
    "?le","?ge","?lt","?gt","?ne","?eq","**","//"
};

static void lexicalError(const std::string &msg, int line) {
    std::cerr << "LEXICAL ERROR: " << msg << " at line " << line << std::endl;
    std::exit(1);
}

static bool isOperatorStart(char c) {
    const std::string ops = ":;+-=(){}[]";
    return ops.find(c) != std::string::npos || c == '?' || c == '*' || c == '/';
}

void initScanner(std::istream &in) {
    tokens.clear();

    std::string line;
    int lineno = 0;
    while (std::getline(in, line)) {
        lineno++;
        size_t i = 0;
        while (i < line.size()) {
            char c = line[i];

            // Skip whitespace
            if (std::isspace((unsigned char)c)) { i++; continue; }

            if (c == '@') {
                // consume until next @ or EOF of file; comments can span lines
                bool closed = false;
                // search rest of current line first
                size_t j = i+1;
                while (true) {
                    // search in current line
                    while (j < line.size()) {
                        if (line[j] == '@') { closed = true; i = j+1; break; }
                        j++;
                    }
                    if (closed) break;
                    // not closed on this line: read next line
                    if (!std::getline(in, line)) {
                        lexicalError("Unterminated comment", lineno);
                    }
                    lineno++;
                    j = 0;
                }
                continue;
            }

            // Identifiers: start with letter 'x' then letters/digits/underscore, up to 8 significant
            if (std::isalpha((unsigned char)c)) {
                size_t j = i;
                std::string ident;
                while (j < line.size() && (std::isalnum((unsigned char)line[j]) || line[j] == '_')) {
                    ident.push_back(line[j]);
                    j++;
                }
                // Keywords are case-sensitive: match the identifier string exactly
                if (keywords.count(ident)) {
                    tokens.push_back({TokenGroup::KEYWORD, ident, lineno});
                    i = j;
                    continue;
                }

                // Identifiers must begin with lowercase x
                if (ident.size() > 0 && ident[0] == 'x') {
                    // enforce up to 8 significant characters
                    if (ident.size() > 8) {
                        lexicalError("Identifier too long: '" + ident + "'", lineno);
                    }
                    tokens.push_back({TokenGroup::IDENTIFIER, ident, lineno});
                    i = j;
                    continue;
                }

                // If it's a word that is not a keyword and not an identifier starting with x -> lexical error
                lexicalError("Invalid identifier or keyword: '" + ident + "'", lineno);
            }

            // Numbers: sequence of digits, up to 8 significant, no sign, no decimal
            if (std::isdigit((unsigned char)c)) {
                size_t j = i;
                std::string num;
                while (j < line.size() && std::isdigit((unsigned char)line[j])) {
                    num.push_back(line[j]);
                    j++;
                }
                if (num.size() > 8) lexicalError("Number too long: '" + num + "'", lineno);
                tokens.push_back({TokenGroup::NUMBER, num, lineno});
                i = j;
                continue;
            }

            // Operators and punctuation
            if (isOperatorStart(c)) {
                // Check multi-char operators: ?xx, **, //
                if (c == '?') {
                    // need two more letters
                    if (i + 2 < line.size()) {
                        std::string op = line.substr(i, 3);
                        if (multi_ops.count(op)) {
                            tokens.push_back({TokenGroup::OPERATOR, op, lineno});
                            i += 3;
                            continue;
                        }
                    }
                    lexicalError(std::string("Unknown operator starting with ? at '") + line.substr(i, std::min<size_t>(3, line.size()-i)) + "'", lineno);
                }

                if (c == '*') {
                    if (i + 1 < line.size() && line[i+1] == '*') {
                        tokens.push_back({TokenGroup::OPERATOR, "**", lineno});
                        i += 2; continue;
                    }
                }
                if (c == '/') {
                    if (i + 1 < line.size() && line[i+1] == '/') {
                        tokens.push_back({TokenGroup::OPERATOR, "//", lineno});
                        i += 2; continue;
                    }
                }

                // New: accept the spaced operator "= =" as a single operator token.
                if (c == '=') {
                    size_t j = i + 1;
                    // allow any amount of whitespace between the two '=' characters
                    while (j < line.size() && std::isspace((unsigned char)line[j])) j++;
                    if (j < line.size() && line[j] == '=') {
                        tokens.push_back({TokenGroup::OPERATOR, "= =", lineno});
                        i = j + 1;
                        continue;
                    }
                }

                // Single char operators or delimiters
                std::string s(1, c);
                const std::string delims = "(){}[]:;";
                if (delims.find(c) != std::string::npos) {
                    tokens.push_back({TokenGroup::DELIMITER, s, lineno});
                    i++; continue;
                }
                // single-char operators left
                if (std::string("+-=").find(c) != std::string::npos) {
                    tokens.push_back({TokenGroup::OPERATOR, s, lineno});
                    i++; continue;
                }
            }

            // invalid character
            lexicalError(std::string("Invalid character: '") + c + "'", lineno);
        }
    }

    // Add EOF token
    tokens.push_back({TokenGroup::END_OF_FILE, "", lineno+1});
}

Token scanner() {
    if (tokens.empty()) {
        return {TokenGroup::END_OF_FILE, "", 0};
    }
    Token t = tokens.front();
    tokens.erase(tokens.begin());
    return t;
}

void testScanner(std::istream &in) {
    initScanner(in);
    while (true) {
        Token t = scanner();
        if (t.group == TokenGroup::END_OF_FILE) {
            std::cout << "Token: " << tokenGroupName(t.group) << " Line: " << t.line << std::endl;
            break;
        }
        std::cout << "Token: " << tokenGroupName(t.group) << " Instance: " << t.instance << " Line: " << t.line << std::endl;
    }
}