#ifndef PARSER_H
    #define PARSER_H
    #include "lexer.h"
    #include <cstdio>
    #include <memory>
    #include <string>
    #include <utility>
    #include <vector>
    struct Stmt
    {
        virtual ~Stmt() = default;
    };
    struct Expr
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
    struct BinaryExpr : Expr
    {
        std::unique_ptr<Expr> left;
        std::uint8_t op;
        std::unique_ptr<Expr> right;
        BinaryExpr(std::unique_ptr<Expr> left, std::uint8_t op, std::unique_ptr<Expr> right) : left(std::move(left)), op(std::move(op)), right(std::move(right))
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
    class Parser
    {
        private:
            std::vector<Token> tokens;
            std::string source;
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
            std::unique_ptr<Expr> parseExpr();
            std::unique_ptr<Expr> parseExponentialExpr();
            std::unique_ptr<Expr> parseMultiplicativeExpr();
            std::unique_ptr<Expr> parseAdditiveExpr();
            std::unique_ptr<Expr> parsePrimaryExpr();
            bool isFunctionDeclaration() const;
            Token peek(unsigned n) const;
        public:
            Parser(std::vector<Token> tokens, std::string source = "");
            ~Parser();
            BlockStmt parse();
    };
#endif