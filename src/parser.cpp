/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "include/parser.h"
#include "include/error.h"
std::string sourceLineSnippet(const std::string& source, unsigned line)
{
    if (source.empty())
    {
        return "";
    }
    std::size_t lineStart = 0;
    unsigned currentLine = 1;
    for (std::size_t i = 0; i < source.size() && currentLine < line; ++i)
    {
        if (source[i] == '\n')
        {
            ++currentLine;
            lineStart = i + 1;
        }
    }
    if (currentLine != line)
    {
        return "";
    }
    std::size_t lineEnd = source.find('\n', lineStart);
    if (lineEnd == std::string::npos)
    {
        lineEnd = source.size();
    }
    return source.substr(lineStart, lineEnd - lineStart);
}
template<typename NodeType>
std::unique_ptr<NodeType> withLocation(std::unique_ptr<NodeType> node, unsigned line, unsigned column, std::string snippet, std::string fileName)
{
    node->line = line;
    node->column = column;
    node->snippet = std::move(snippet);
    node->fileName = std::move(fileName);
    return node;
}
template<typename NodeType>
std::unique_ptr<NodeType> withLocation(std::unique_ptr<NodeType> node, const Token& token, const std::string& source, const std::string& fileName)
{
    return withLocation(std::move(node), token.row, token.col, sourceLineSnippet(source, token.row), fileName);
}
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
    if (typeKind != TokenKind::tok_ident && typeKind != TokenKind::tok_predefined_type)
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
        error(1, ErrorType::err_unexpected_token, "Unexpected token: " + tk.value, tk.col, tk.row, this->currentLineSnippet(), this->fileName);
    }
    return tk;
}
std::string Parser::currentLineSnippet() const
{
    const Token current = this->currentToken();
    return sourceLineSnippet(this->source, current.row);
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
// MemberExpr
// CallExpr
// ComparisonExpr
// AdditiveExpr
// MultiplicativeExpr
// ExponentialExpr
// UnaryExpr
// PrimaryExpr
std::unique_ptr<Expr> Parser::parseCallExpr()
{
    const Token tk = this->currentToken();
    if (tk.kind != TokenKind::tok_ident || this->peek(1).kind != TokenKind::tok_open_paren)
    {
        return this->parsePrimaryExpr();
    }
    this->advance();
    const std::string name = tk.value;
    this->expect(TokenKind::tok_open_paren);
    std::vector<std::unique_ptr<Expr>> args = {};
    while (this->currentTokenKind() != TokenKind::tok_closed_paren)
    {
        args.push_back(std::move(this->parseExpr()));
        if (this->currentTokenKind() != TokenKind::tok_closed_paren)
        {
            this->expect(TokenKind::tok_comma);
        }
    }
    this->advance();
    // TODO: add generics
    return withLocation(std::make_unique<CallExpr>(CallExpr{
        name,
        std::move(args)
    }), tk, this->source, this->fileName);
}
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
        const unsigned line = left->line;
        const unsigned column = left->column;
        const std::string snippet = left->snippet;
        left = withLocation(std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        }), line, column, snippet, this->fileName);
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
        const unsigned line = left->line;
        const unsigned column = left->column;
        const std::string snippet = left->snippet;
        left = withLocation(std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        }), line, column, snippet, this->fileName);
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
        const unsigned line = left->line;
        const unsigned column = left->column;
        const std::string snippet = left->snippet;
        left = withLocation(std::make_unique<BinaryExpr>(BinaryExpr{
            std::move(left),
            op,
            std::move(right)
        }), line, column, snippet, this->fileName);
    }
    return left;
}
std::unique_ptr<Expr> Parser::parsePrimaryExpr()
{
    const Token token = this->currentToken();
    const std::uint8_t tk = token.kind;
    switch (tk)
    {
        case TokenKind::tok_ident:
            if (this->peek(1).kind == TokenKind::tok_open_paren)
            {
                return this->parseCallExpr();
            }
            this->advance();
            return withLocation(std::make_unique<SymbolExpr>(SymbolExpr{token.value}), token, this->source, this->fileName);
        case TokenKind::tok_int:
            this->advance();
            return withLocation(std::make_unique<IntegerExpr>(IntegerExpr{std::stoi(token.value)}), token, this->source, this->fileName);
        case TokenKind::tok_float:
            this->advance();
            return withLocation(std::make_unique<FloatExpr>(FloatExpr{std::stod(token.value)}), token, this->source, this->fileName);
        case TokenKind::tok_string:
            this->advance();
            return withLocation(std::make_unique<StringExpr>(StringExpr{token.value}), token, this->source, this->fileName);
        case TokenKind::tok_false:
        case TokenKind::tok_true:
            this->advance();
            return withLocation(std::make_unique<BooleanExpr>(BooleanExpr{token.kind - TokenKind::tok_false == 1}), token, this->source, this->fileName);
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
            error(1, ErrorType::err_unexpected_token, "Unexpected token: " + current.value, current.col, current.row, snippet, this->fileName);
            return nullptr;
        }
    }
}
std::unique_ptr<TypeStmt> Parser::parseTypeStmt()
{
    const Token tk = this->advance();
    if (tk.kind != TokenKind::tok_predefined_type && tk.kind != TokenKind::tok_ident)
    {
        error(1, ErrorType::err_unexpected_token, "Did not expect token \"" + tk.value + "\" in variable definition!", tk.col, tk.row, currentLineSnippet(), this->fileName);
    }
    if (this->peek(1).kind == TokenKind::tok_open_paren)
    {
        // TODO: add generics
    }
    return withLocation(std::make_unique<TypeStmt>(TypeStmt{
        tk.value,
        {}
    }), tk, this->source, this->fileName);
}
std::unique_ptr<VarDeclarationStmt> Parser::parseVarDeclarationStmt()
{
    const Token start = this->currentToken();
    const bool isConstant = this->currentTokenKind() == TokenKind::tok_const;
    if (isConstant)
    {
        this->advance();
    }
    std::unique_ptr<TypeStmt> type = this->parseTypeStmt();
    const std::string ident = this->expect(TokenKind::tok_ident).value;
    this->expect(TokenKind::tok_eq);
    std::unique_ptr<Expr> expr = this->parseExpr();
    if (this->currentTokenKind() != TokenKind::tok_eof && this->currentTokenKind() != TokenKind::tok_dedent)
    {
        this->expect(TokenKind::tok_newline);
    }
    return withLocation(std::make_unique<VarDeclarationStmt>(VarDeclarationStmt{
        isConstant,
        ident,
        std::move(type),
        std::move(expr)
    }), start, this->source, this->fileName);
}
std::unique_ptr<FuncDeclarationStmt> Parser::parseFuncDeclarationStmt()
{
    const Token start = this->currentToken();
    if (this->currentTokenKind() == TokenKind::tok_const)
    {
        error(1, ErrorType::err_unexpected_token, "Functions cannot be made constant!", this->currentToken().col, this->currentToken().row, currentLineSnippet(), this->fileName);
        this->advance();
    }
    std::unique_ptr<TypeStmt> type = this->parseTypeStmt();
    const std::string name = this->expect(TokenKind::tok_ident).value;
    this->expect(TokenKind::tok_open_paren);
    std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args = {};
    while (this->currentTokenKind() != TokenKind::tok_closed_paren)
    {
        std::unique_ptr<TypeStmt> argtype = this->parseTypeStmt();
        const std::string argname = this->expect(TokenKind::tok_ident).value;
        if (this->currentTokenKind() != TokenKind::tok_closed_paren)
        {
            this->expect(TokenKind::tok_comma);
        }
        args.push_back(std::make_pair(std::move(argtype), argname));
    }
    this->advance();
    // TODO: add generics
    if (this->currentTokenKind() == TokenKind::tok_open_paren)
    {}
    if (this->currentTokenKind() != TokenKind::tok_newline)
    {
        return withLocation(std::make_unique<FuncDeclarationStmt>(FuncDeclarationStmt{
            name,
            std::move(type),
            {},
            std::move(args),
            withLocation(std::make_unique<BlockStmt>(BlockStmt{
                {}
            }), start, this->source, this->fileName)
        }), start, this->source, this->fileName);
    }
    this->advance();
    if (this->currentTokenKind() != TokenKind::tok_indent)
    {
        return withLocation(std::make_unique<FuncDeclarationStmt>(FuncDeclarationStmt{
            name,
            std::move(type),
            {},
            std::move(args),
            withLocation(std::make_unique<BlockStmt>(BlockStmt{
                {}
            }), start, this->source, this->fileName)
        }), start, this->source, this->fileName);
    }
    const Token bodyStart = this->currentToken();
    this->advance();
    std::unique_ptr<BlockStmt> stmt = withLocation(std::make_unique<BlockStmt>(BlockStmt{{}}), bodyStart, this->source, this->fileName);
    while (this->currentTokenKind() != TokenKind::tok_dedent && this->currentTokenKind() != TokenKind::tok_eof)
    {
        std::unique_ptr<Stmt> statement = this->parseStmt();
        if (!statement)
        {
            break;
        }
        stmt->body.push_back(std::move(statement));
    }
    if (this->currentTokenKind() == TokenKind::tok_dedent)
    {
        this->advance();
    }
    return withLocation(std::make_unique<FuncDeclarationStmt>(FuncDeclarationStmt{
        name,
        std::move(type),
        {},
        std::move(args),
        std::move(stmt)
    }), start, this->source, this->fileName);
}
std::unique_ptr<ReturnStmt> Parser::parseReturnStmt()
{
    const Token start = this->advance();
    std::unique_ptr<Expr> value;
    if (this->currentTokenKind() != TokenKind::tok_newline && this->currentTokenKind() != TokenKind::tok_dedent && this->currentTokenKind() != TokenKind::tok_eof)
    {
        value = this->parseExpr();
    }
    return withLocation(std::make_unique<ReturnStmt>(ReturnStmt{
        std::move(value)
    }), start, this->source, this->fileName);
}
std::unique_ptr<ImportStmt> Parser::parseImportStmt()
{
    const Token start = this->advance();
    const std::string importPath = this->expect(TokenKind::tok_string).value;
    return withLocation(std::make_unique<ImportStmt>(ImportStmt{
        importPath.empty() ? "" : importPath.substr(1)
    }), start, this->source, this->fileName);
}
std::unique_ptr<Stmt> Parser::parseStmt()
{
    switch (this->currentTokenKind())
    {
        case TokenKind::tok_newline:
        case TokenKind::tok_indent:
            this->advance();
            if (this->currentTokenKind() == TokenKind::tok_dedent || this->currentTokenKind() == TokenKind::tok_eof)
            {
                return nullptr;
            }
            return this->parseStmt();
        case TokenKind::tok_const:
        case TokenKind::tok_predefined_type:
            if (this->isFunctionDeclaration())
            {
                return this->parseFuncDeclarationStmt();
            }
            return this->parseVarDeclarationStmt();
        case TokenKind::tok_return:
            return this->parseReturnStmt();
        case TokenKind::tok_import:
            return this->parseImportStmt();
        default:
        {
            std::unique_ptr<Expr> expr = this->parseExpr();
            if (!expr)
            {
                return nullptr;
            }
            const unsigned line = expr->line;
            const unsigned column = expr->column;
            const std::string snippet = expr->snippet;
            return withLocation(std::make_unique<ExpressionStmt>(ExpressionStmt{
                std::move(expr)
            }), line, column, snippet, this->fileName);
        }
    }
}
BlockStmt Parser::parse()
{
    const Token start = this->currentToken();
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
    BlockStmt block{std::move(body)};
    block.line = start.row;
    block.column = start.col;
    block.snippet = sourceLineSnippet(this->source, start.row);
    block.fileName = this->fileName;
    return block;
}
Parser::Parser(std::vector<Token> tokens, std::string source, std::string fileName)
{
    this->tokens = std::move(tokens);
    this->source = std::move(source);
    this->fileName = std::move(fileName);
    this->pos = 0;
}
Parser::~Parser()
{}