#ifndef CODEGEN_H
    #define CODEGEN_H
    #include "parser.h"
    #include <memory>
    #include <string>
    #include <unordered_set>
    #include <utility>
    #include <vector>
    struct Core
    {
        std::unordered_set<std::string> dependencies;
        Core();
        std::string get_result() const;
    };
    std::string t_int();
    void req(Core& core, const std::string& dependency);
    std::string decl_func(const std::string& return_type, const std::string& name, const std::vector<std::pair<std::string, std::string>>& args, const std::string& content);
    std::string decl_ret(const std::string& value);
    std::string decl_add(const std::string& lhs, const std::string& rhs);
    std::string decl_sub(const std::string& lhs, const std::string& rhs);
    std::string decl_mult(const std::string& lhs, const std::string& rhs);
    std::string decl_div(const std::string& lhs, const std::string& rhs);
    std::string decl_mod(const std::string& lhs, const std::string& rhs);
    std::string decl_pow(Core& core, const std::string& lhs, const std::string& rhs);
    std::string decl_root(Core& core, const std::string& lhs, const std::string& rhs);
    std::string decl_var(Core& core, const bool constant, const std::string& name, const std::string& type, const std::string& value);
    std::string generate(Core& core, BlockStmt block);
    std::string evaluate_stmt(Core& core, std::unique_ptr<Stmt> stmt);
    std::string evaluate_type_stmt(Core& core, std::unique_ptr<TypeStmt> stmt);
    std::string evaluate_expr(Core& core, std::unique_ptr<Expr> expr);
    std::string evaluate_binary_expr(Core& core, BinaryExpr* expr);
#endif