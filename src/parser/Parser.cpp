#include "Parser.h"

#include <sstream>
#include <string>

namespace {
std::string posString(size_t pos) {
    std::ostringstream oss;
    oss << "pos " << pos;
    return oss.str();
}

std::string unexpectedMessage(const char* expected, const Token& tok) {
    if (tok.type == TokenType::Invalid) {
        std::ostringstream oss;
        oss << tok.error << " at " << posString(tok.position);
        return oss.str();
    }
    if (tok.type == TokenType::End) {
        std::ostringstream oss;
        oss << "unexpected end of input at " << posString(tok.position);
        return oss.str();
    }
    std::ostringstream oss;
    oss << "expected " << expected << " at " << posString(tok.position);
    return oss.str();
}
} // namespace

bool Parser::parse(std::string_view input, ParsedExpr& out, std::string& error) const {
    Lexer lexer(input);
    Token lhs = lexer.next();
    if (lhs.type != TokenType::Integer) {
        error = unexpectedMessage("integer", lhs);
        return false;
    }

    Token op = lexer.next();
    if (op.type != TokenType::Operator) {
        error = unexpectedMessage("operator", op);
        return false;
    }

    Token rhs = lexer.next();
    if (rhs.type != TokenType::Integer) {
        error = unexpectedMessage("integer", rhs);
        return false;
    }

    Token end = lexer.next();
    if (end.type != TokenType::End) {
        error = unexpectedMessage("end", end);
        return false;
    }

    out.lhs = std::string(lhs.lexeme);
    out.op = op.op;
    out.rhs = std::string(rhs.lexeme);
    return true;
}
