#include "include/codegen.h"
#include "include/error.h"
Core::Core() : dependencies{} {}
std::string Core::get_result() const
{
    std::string includes;
    includes.reserve(dependencies.size() * 16);
    for (const std::string& dependency : dependencies)
    {
        includes += "#include<" + dependency + ">\n";
    }
    return includes;
}
std::string t_int()
{
    return "int";
}
void req(Core& core, const std::string& dependency)
{
    auto dependencyCheck = core.dependencies.find(dependency);
    if (dependencyCheck == core.dependencies.end())
    {
        core.dependencies.insert(dependency);
    }
}
std::string decl_func(const std::string& return_type, const std::string& name, const std::vector<std::pair<std::string, std::string>>& args, const std::string& content)
{
    std::string stringifiedArgs;
    for (const auto& arg : args)
    {
        stringifiedArgs += arg.first + " " + arg.second + ",";
    }
    return return_type + " " + name + "(" + stringifiedArgs + "){" + content + "}";
}
std::string decl_ret(const std::string& value)
{
    return "return " + value + ";";
}
std::string decl_add(const std::string& lhs, const std::string& rhs)
{
    return "(" + lhs + "+" + rhs + ")";
}
std::string decl_sub(const std::string& lhs, const std::string& rhs)
{
    return "(" + lhs + "-" + rhs + ")";
}
std::string decl_mult(const std::string& lhs, const std::string& rhs)
{
    return "(" + lhs + "*" + rhs + ")";
}
std::string decl_div(const std::string& lhs, const std::string& rhs)
{
    return "(" + lhs + "/" + rhs + ")";
}
std::string decl_mod(const std::string& lhs, const std::string& rhs)
{
    return "(" + lhs + "%" + rhs + ")";
}
std::string decl_pow(Core& core, const std::string& lhs, const std::string& rhs)
{
    req(core, "cmath");
    return "(std::pow(" + lhs + "," + rhs + "))";
}
std::string decl_root(Core& core, const std::string& lhs, const std::string& rhs)
{
    req(core, "cmath");
    return "(std::pow(" + lhs + ",1.0/" + rhs + "))";
}
std::string decl_var(Core& core, const bool constant, const std::string& name, const std::string& type, const std::string& value)
{
    return (constant ? "const " : "") + type + " " + name + "=" + value;
}
std::string evaluate_type_stmt(Core& core, std::unique_ptr<TypeStmt> stmt)
{
    // Temporarily just output the name of the type
    return stmt->name;
}
std::string evaluate_var_decl_stmt(Core& core, std::unique_ptr<VarDeclarationStmt> stmt)
{
    return decl_var(core, stmt->constant, stmt->identifier, evaluate_type_stmt(core, std::move(stmt->type)), evaluate_expr(core, std::move(stmt->value)));
}
std::string generate(Core& core, BlockStmt block)
{
    std::string res;
    res.reserve(block.body.size() * 16);
    for (auto& ptr : block.body)
    {
        res += evaluate_stmt(core, std::move(ptr));
    }
    return res;
}
std::string evaluate_stmt(Core& core, std::unique_ptr<Stmt> stmt)
{
    if (auto expr = dynamic_cast<ExpressionStmt*>(stmt.get()))
    {
        return evaluate_expr(core, std::move(expr->expression));
    }
    else if (auto var = dynamic_cast<VarDeclarationStmt*>(stmt.get()))
    {
        stmt.release();
        return evaluate_var_decl_stmt(core, std::unique_ptr<VarDeclarationStmt>(var)) + ";";
    }
    return "UNIMPLEMENTED";
}
std::string evaluate_expr(Core& core, std::unique_ptr<Expr> expr)
{
    if (auto expr2 = dynamic_cast<BinaryExpr*>(expr.get()))
    {
        return evaluate_binary_expr(core, expr2);
    }
    if (auto expr2 = dynamic_cast<IntegerExpr*>(expr.get()))
    {
        return std::to_string(expr2->value);
    }
    if (auto expr2 = dynamic_cast<FloatExpr*>(expr.get()))
    {
        return std::to_string(expr2->value);
    }
    if (auto expr2 = dynamic_cast<SymbolExpr*>(expr.get()))
    {
        return expr2->value;
    }
    return "UNIMPLEMENTED";
}
std::string evaluate_binary_expr(Core& core, BinaryExpr* expr)
{
    switch (expr->op)
    {
        case TokenKind::tok_plus:
            return decl_add(evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        case TokenKind::tok_minus:
            return decl_sub(evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        case TokenKind::tok_star:
            return decl_mult(evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        case TokenKind::tok_slash:
            return decl_div(evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        case TokenKind::tok_percent:
            return decl_mod(evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        case TokenKind::tok_caret:
            return decl_pow(core, evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
        default: // TokenKind::tok_hash
            return decl_root(core, evaluate_expr(core, std::move(expr->left)), evaluate_expr(core, std::move(expr->right)));
    }
}
std::string decl_block(std::vector<std::string> block)
{
    std::string res;
    for (const std::string str : block)
    {
        res += str;
    }
    return res;
}