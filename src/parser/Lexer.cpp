#include "Lexer.h"

#include <cctype>

Lexer::Lexer(std::string_view input) : input_(input), pos_(0) {}

void Lexer::skipSpaces() {
    while (pos_ < input_.size() &&
           std::isspace(static_cast<unsigned char>(input_[pos_]))) {
        ++pos_;
    }
}

Token Lexer::next() {
    skipSpaces();
    size_t start = pos_;
    if (pos_ >= input_.size()) {
        return Token{TokenType::End, std::string_view{}, pos_, 0, {}};
    }

    const auto isDigit = [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) != 0;
    };

    char c = input_[pos_];
    if (isDigit(c) ||
        ((c == '+' || c == '-') && pos_ + 1 < input_.size() &&
         isDigit(input_[pos_ + 1]))) {
        ++pos_;
        while (pos_ < input_.size() && isDigit(input_[pos_])) {
            ++pos_;
        }
        return Token{TokenType::Integer, input_.substr(start, pos_ - start), start, 0,
                     {}};
    }

    if (c == '+' || c == '-' || c == '*' || c == '%') {
        ++pos_;
        return Token{TokenType::Operator, input_.substr(start, 1), start, c, {}};
    }

    ++pos_;
    Token tok{TokenType::Invalid, input_.substr(start, 1), start, 0, {}};
    tok.error = "invalid character";
    return tok;
}
