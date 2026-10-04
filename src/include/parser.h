/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#ifndef PARSER_H
    #define PARSER_H
    #include "lexer.h"
    #include <cstdio>
    #include <memory>
    #include <string>
    #include <utility>
    #include <vector>
    struct AstNode
    {
        unsigned line = 0;
        unsigned column = 0;
        std::string snippet;
        std::string fileName;
        virtual ~AstNode() = default;
    };
    struct Stmt : AstNode
    {
        virtual ~Stmt() = default;
    };
    struct Expr : AstNode
    {
        virtual ~Expr() = default;
    };
    struct IntegerExpr : Expr
    {
        int value;
        IntegerExpr(int value) : value(value)
        {}
    };
    struct FloatExpr : Expr
    {
        double value;
        FloatExpr(double value) : value(value)
        {}
    };
    struct StringExpr : Expr
    {
        std::string value;
        StringExpr(std::string value) : value(std::move(value))
        {}
    };
    struct SymbolExpr : Expr
    {
        std::string value;
        SymbolExpr(std::string value) : value(std::move(value))
        {}
    };
    struct BooleanExpr : Expr
    {
        bool value;
        BooleanExpr(bool value) : value(value)
        {}
    };
    struct BinaryExpr : Expr
    {
        std::unique_ptr<Expr> left;
        std::uint8_t op;
        std::unique_ptr<Expr> right;
        BinaryExpr(std::unique_ptr<Expr> left, std::uint8_t op, std::unique_ptr<Expr> right) : left(std::move(left)), op(std::move(op)), right(std::move(right))
        {}
    };
    struct CallExpr : Expr
    {
        std::string name;
        std::vector<std::unique_ptr<Expr>> args;
        CallExpr(std::string name, std::vector<std::unique_ptr<Expr>> args) : name(name), args(std::move(args))
        {}
    };
    struct BlockStmt : Stmt
    {
        std::vector<std::unique_ptr<Stmt>> body;
        BlockStmt(std::vector<std::unique_ptr<Stmt>> body) : body(std::move(body))
        {}
    };
    struct ExpressionStmt : Stmt
    {
        std::unique_ptr<Expr> expression;
        ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression))
        {}
    };
    struct TypeStmt : Stmt
    {
        std::string name;
        std::vector<std::unique_ptr<TypeStmt>> generics;
        TypeStmt(std::string name, std::vector<std::unique_ptr<TypeStmt>> generics) : name(name), generics(std::move(generics))
        {}
    };
    struct VarDeclarationStmt : Stmt
    {
        bool constant;
        std::string identifier;
        std::unique_ptr<TypeStmt> type;
        std::unique_ptr<Expr> value;
        VarDeclarationStmt(bool constant, std::string identifier, std::unique_ptr<TypeStmt> type, std::unique_ptr<Expr> value) : constant(constant), identifier(identifier), type(std::move(type)), value(std::move(value))
        {}
    };
    enum VarAssignmentKind
    {
        equals,
        plus_equals,
        minus_equals,
        star_equals,
        slash_equals,
        percent_equals,
        caret_equals,
        hash_equals
    };
    struct VarAssigmentStmt : Stmt
    {
        std::string identifier;
        std::uint8_t op;
        std::unique_ptr<Expr> value;
        VarAssigmentStmt(std::string identifier, std::uint8_t op, std::unique_ptr<Expr> value) : identifier(identifier), op(op), value(std::move(value))
        {}
    };
    struct FuncDeclarationStmt : Stmt
    {
        std::string identifier;
        std::unique_ptr<TypeStmt> type;
        std::vector<std::string> generics;
        std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args;
        std::unique_ptr<BlockStmt> block;
        FuncDeclarationStmt(std::string identifier, std::unique_ptr<TypeStmt> type, std::vector<std::string> generics, std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args, std::unique_ptr<BlockStmt> block) : identifier(identifier), type(std::move(type)), args(std::move(args)), block(std::move(block))
        {}
    };
    struct ReturnStmt : Stmt
    {
        std::unique_ptr<Expr> value;
        ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value))
        {}
    };
    struct ImportStmt : Stmt
    {
        std::string from;
        ImportStmt(std::string from) : from(from)
        {}
    };
    class Parser
    {
        private:
            std::vector<Token> tokens;
            std::string source;
            std::string fileName;
            unsigned pos;
            Token currentToken() const;
            std::uint8_t currentTokenKind() const;
            Token advance();
            Token expect(std::uint8_t kind);
            bool hasTokens() const;
            std::string currentLineSnippet() const;
            std::unique_ptr<Stmt> parseStmt();
            std::unique_ptr<TypeStmt> parseTypeStmt();
            std::unique_ptr<VarDeclarationStmt> parseVarDeclarationStmt();
            std::unique_ptr<FuncDeclarationStmt> parseFuncDeclarationStmt();
            std::unique_ptr<ReturnStmt> parseReturnStmt();
            std::unique_ptr<ImportStmt> parseImportStmt();
            std::unique_ptr<Expr> parseExpr();
            std::unique_ptr<Expr> parsePrimaryExpr();
            std::unique_ptr<Expr> parseCallExpr();
            std::unique_ptr<Expr> parseExponentialExpr();
            std::unique_ptr<Expr> parseMultiplicativeExpr();
            std::unique_ptr<Expr> parseAdditiveExpr();
            bool isFunctionDeclaration() const;
            Token peek(unsigned n) const;
        public:
            Parser(std::vector<Token> tokens, std::string source = "", std::string fileName = "");
            ~Parser();
            BlockStmt parse();
    };
#endif