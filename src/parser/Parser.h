// Parser consumes lexer tokens and builds ParsedExpr without touching BigInt.
#pragma once

#include <string>
#include <string_view>

#include "Lexer.h"

struct ParsedExpr {
    std::string lhs;
    char op;
    std::string rhs;
};

class Parser {
public:
    bool parse(std::string_view input, ParsedExpr& out, std::string& error) const;
};
