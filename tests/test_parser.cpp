#include <cassert>
#include <iostream>
#include <string>

#include "../src/parser/Parser.h"

void expectParse(const std::string& input, const std::string& lhs, char op,
                 const std::string& rhs) {
    Parser parser;
    ParsedExpr expr;
    std::string error;
    assert(parser.parse(input, expr, error));
    assert(expr.lhs == lhs);
    assert(expr.op == op);
    assert(expr.rhs == rhs);
}

void expectFail(const std::string& input, const std::string& pos_fragment) {
    Parser parser;
    ParsedExpr expr;
    std::string error;
    assert(!parser.parse(input, expr, error));
    assert(error.find(pos_fragment) != std::string::npos);
}

int main() {
    expectParse("123 + 456", "123", '+', "456");
    expectParse("   -0012   *   +34 ", "-0012", '*', "+34");
    expectParse("5 % 2", "5", '%', "2");

    expectFail("123", "pos 3");
    expectFail("1 & 2", "pos 2");
    expectFail("abc", "pos 0");
    expectFail("1 + ", "pos 4");
    expectFail("1 2 + 3", "pos 2");

    std::cout << "parser tests passed\n";
    return 0;
}
