#include "include/parser.h"
#include "include/error.h"
Token Parser::peek(unsigned n) const
{
    std::size_t index = this->pos + n;
    if (index < this->tokens.size())
    {
        return this->tokens[index];
    }
    return this->tokens[this->tokens.size() - 1];
}
bool Parser::isFunctionDeclaration() const
{
    unsigned offset = 0;
    if (this->peek(offset).kind == TokenKind::tok_const)
    {
        ++offset;
    }
    const std::uint8_t typeKind = this->peek(offset).kind;
    if (typeKind != TokenKind::tok_ident && typeKind != TokenKind::tok_int_keyword && typeKind != TokenKind::tok_float_keyword && typeKind != TokenKind::tok_string_keyword)
    {
        return false;
    }
    ++offset;

    if (this->peek(offset).kind == TokenKind::tok_lt)
    {
        unsigned genericDepth = 0;
        do
        {
            const std::uint8_t kind = this->peek(offset).kind;
            if (kind == TokenKind::tok_lt)
            {
                ++genericDepth;
            }
            else if (kind == TokenKind::tok_gt)
            {
                --genericDepth;
            }
            ++offset;
        } while (genericDepth > 0 && this->peek(offset - 1).kind != TokenKind::tok_eof);
    }

    return this->peek(offset).kind == TokenKind::tok_ident && this->peek(offset + 1).kind == TokenKind::tok_open_paren;
}
Token Parser::currentToken() const
{
    return this->tokens[this->pos];
}
std::uint8_t Parser::currentTokenKind() const
{
    return this->currentToken().kind;
}
Token Parser::advance()
{
    const Token tk = this->currentToken();
    ++(this->pos);
    return tk;
}
Token Parser::expect(std::uint8_t kind)
{
    const Token tk = this->advance();
    if (tk.kind != kind)
    {
        error(1, ErrorType::err_unexpected_token, "Unexpected token: " + tk.value, tk.col, tk.row, this->currentLineSnippet());
    }
    return tk;
}
std::string Parser::currentLineSnippet() const
{
    if (this->source.empty())
    {
        return "";
    }
    const Token current = this->currentToken();
    std::size_t lineStart = 0;
    unsigned currentRow = 1;
    for (std::size_t i = 0; i < this->source.size(); ++i)
    {
        if (currentRow == current.row)
        {
            lineStart = i;
            break;
        }
        if (this->source[i] == '\n')
        {
            ++currentRow;
        }
    }
    std::size_t lineEnd = this->source.find('\n', lineStart);
    if (lineEnd == std::string::npos)
    {
        lineEnd = this->source.size();
    }
    return this->source.substr(lineStart, lineEnd - lineStart);
}
bool Parser::hasTokens() const
{
    return this->pos < this->tokens.size() && this->currentTokenKind() != TokenKind::tok_eof;
}
std::unique_ptr<Expr> Parser::parseExpr()
{
    return this->parseAdditiveExpr();
}
// == Prescidence: ==
// AssignmentExpr
// MemberExpr
// FunctionCallExpr
// ComparisonExpr
// AdditiveExpr
// MultiplicativeExpr
// ExponentialExpr
// UnaryExpr
// PrimaryExpr
std::unique_ptr<Expr> Parser::parseExponentialExpr()
{
    std::unique_ptr<Expr> left = this->parsePrimaryExpr();
    if (!left)
    {
        return nullptr;
    }
    while (this->currentTokenKind() == TokenKind::tok_caret || this->currentTokenKind() == TokenKind::tok_hash)
    {
        const std::uint8_t op = this->advance().kind;
        std::unique_ptr<Expr> right = this->parsePrimaryExpr();
        if (!right)
        {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        });
    }
    return left;
}
std::unique_ptr<Expr> Parser::parseMultiplicativeExpr()
{
    std::unique_ptr<Expr> left = this->parseExponentialExpr();
    if (!left)
    {
        return nullptr;
    }
    while (this->currentTokenKind() == TokenKind::tok_star || this->currentTokenKind() == TokenKind::tok_slash || this->currentTokenKind() == TokenKind::tok_percent)
    {
        const std::uint8_t op = this->advance().kind;
        std::unique_ptr<Expr> right = this->parseExponentialExpr();
        if (!right)
        {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        });
    }
    return left;
}
std::unique_ptr<Expr> Parser::parseAdditiveExpr()
{
    std::unique_ptr<Expr> left = this->parseMultiplicativeExpr();
    if (!left)
    {
        return nullptr;
    }
    while (this->currentTokenKind() == TokenKind::tok_plus || this->currentTokenKind() == TokenKind::tok_minus)
    {
        const std::uint8_t op = this->advance().kind;
        std::unique_ptr<Expr> right = this->parseMultiplicativeExpr();
        if (!right)
        {
            return nullptr;
        }
        left = std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        });
    }
    return left;
}
std::unique_ptr<Expr> Parser::parsePrimaryExpr()
{
    const std::uint8_t tk = this->currentTokenKind();
    switch (tk)
    {
        case TokenKind::tok_ident:
            return std::make_unique<SymbolExpr>(SymbolExpr{this->advance().value});
        case TokenKind::tok_int:
            return std::make_unique<IntegerExpr>(IntegerExpr{std::stoi(this->advance().value)});
        case TokenKind::tok_float:
            return std::make_unique<FloatExpr>(FloatExpr{std::stod(this->advance().value)});
        case TokenKind::tok_open_paren:
        {
            this->advance();
            std::unique_ptr<Expr> value = this->parseExpr();
            this->expect(TokenKind::tok_closed_paren);
            return std::move(value);
        }
        default:
        {
            const Token current = this->currentToken();
            const std::string snippet = this->currentLineSnippet();
            error(1, ErrorType::err_unexpected_token, "Unexpected token: " + current.value, current.col, current.row, snippet);
            return nullptr;
        }
    }
}
std::unique_ptr<TypeStmt> Parser::parseTypeStmt()
{
    const Token tk = this->advance();
    if (tk.kind != TokenKind::tok_int_keyword && tk.kind != TokenKind::tok_float_keyword && tk.kind != TokenKind::tok_string_keyword && tk.kind != TokenKind::tok_ident)
    {
        error(1, ErrorType::err_unexpected_token, "Did not expect token \"" + tk.value + "\" in variable definition!", tk.col, tk.row, currentLineSnippet());
    }
    if (this->peek(1).kind == TokenKind::tok_open_paren)
    {
        // TODO: add generics
    }
    return std::make_unique<TypeStmt>(TypeStmt{
        tk.value,
        {}
    });
}
std::unique_ptr<VarDeclarationStmt> Parser::parseVarDeclarationStmt()
{
    const bool isConstant = this->currentTokenKind() == TokenKind::tok_const;
    if (isConstant)
    {
        this->advance();
    }
    std::unique_ptr<TypeStmt> type = this->parseTypeStmt();
    const std::string ident = this->expect(TokenKind::tok_ident).value;
    this->expect(TokenKind::tok_eq);
    std::unique_ptr<Expr> expr = this->parseExpr();
    if (this->currentTokenKind() != TokenKind::tok_eof)
    {
        this->expect(TokenKind::tok_newline);
    }
    return std::make_unique<VarDeclarationStmt>(VarDeclarationStmt{
        isConstant,
        ident,
        std::move(type),
        std::move(expr)
    });
}
std::unique_ptr<Stmt> Parser::parseStmt()
{
    switch (this->currentTokenKind())
    {
        case TokenKind::tok_newline:
            this->advance();
            return this->parseStmt();
        case TokenKind::tok_const:
        case TokenKind::tok_int_keyword:
        case TokenKind::tok_float_keyword:
        case TokenKind::tok_string_keyword:
            if (this->isFunctionDeclaration())
            {
                // to-be function declarations
                return nullptr;
            }
            return this->parseVarDeclarationStmt();
        default:
        {
            std::unique_ptr<Expr> expr = this->parseExpr();
            if (!expr)
            {
                return nullptr;
            }
            return std::make_unique<ExpressionStmt>(ExpressionStmt{std::move(expr)});
        }
    }
    return nullptr;
}
BlockStmt Parser::parse()
{
    std::vector<std::unique_ptr<Stmt>> body = {};
    while (this->hasTokens())
    {
        std::unique_ptr<Stmt> stmt = this->parseStmt();
        if (!stmt)
        {
            break;
        }
        body.push_back(std::move(stmt));
    }
    return BlockStmt{std::move(body)};
}
Parser::Parser(std::vector<Token> tokens, std::string source)
{
    this->tokens = std::move(tokens);
    this->source = std::move(source);
    this->pos = 0;
}
Parser::~Parser()
{}