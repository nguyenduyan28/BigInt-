#include <iostream>
#include <string>

#include "../bigint/BigInt.h"
#include "../parser/Parser.h"

int main() {
    Parser parser;
    std::string line;

    while (std::getline(std::cin, line)) {
        ParsedExpr expr;
        std::string error;
        if (!parser.parse(line, expr, error)) {
            std::cerr << "parse error: " << error << '\n';
            continue;
        }

        try {
            BigInt lhs = BigInt::fromString(expr.lhs);
            BigInt rhs = BigInt::fromString(expr.rhs);

            BigInt result;
            switch (expr.op) {
            case '+':
                result = lhs + rhs;
                break;
            case '-':
                result = lhs - rhs;
                break;
            case '*':
                result = lhs * rhs;
                break;
            case '%':
                result = lhs % rhs;
                break;
            default:
                std::cerr << "math error: unsupported operator '" << expr.op << "'\n";
                continue;
            }

            std::cout << result.toString() << std::endl;
        } catch (const std::invalid_argument& ex) {
            std::cerr << "number error: " << ex.what() << '\n';
        } catch (const std::runtime_error& ex) {
            std::cerr << "math error: " << ex.what() << '\n';
        }
    }

    return 0;
}
