// Lexer: splits input expression into integer, operator, and end tokens.
#pragma once

#include <string>
#include <string_view>

enum class TokenType {
    Integer,
    Operator,
    End,
    Invalid
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    size_t position;
    char op;            // valid when type == Operator
    std::string error;  // populated when type == Invalid
};

class Lexer {
public:
    explicit Lexer(std::string_view input);

    Token next();

private:
    std::string_view input_;
    size_t pos_;

    void skipSpaces();
};
