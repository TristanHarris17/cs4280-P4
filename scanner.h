#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>
#include "token.h"

void initScanner(std::istream &in);

Token scanner();

void testScanner(std::istream &in);

#endif
