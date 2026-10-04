/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "include/codegen.h"
#include "include/error.h"
#include "include/lexer.h"
namespace
{
    std::string evaluate_import_stmt(Core& core, std::unique_ptr<ImportStmt> stmt);
    std::filesystem::path normalizedFilePath(const std::filesystem::path& path)
    {
        std::error_code errorCode;
        const std::filesystem::path absolutePath = std::filesystem::absolute(path, errorCode);
        if (errorCode)
        {
            return path.lexically_normal();
        }
        const std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(absolutePath, errorCode);
        return (errorCode ? absolutePath : canonicalPath).lexically_normal();
    }
}
std::string Core::get_result() const
{
    std::string includes;
    includes.reserve(dependencies.size() * 16);
    for (const std::string& dependency : this->dependencies)
    {
        includes += "#include<" + dependency + ">\n";
    }
    return includes + this->imports + this->forward_declarations;
}
void req(Core &core, const std::string &dependency)
{
    auto dependencyCheck = core.dependencies.find(dependency);
    if (dependencyCheck == core.dependencies.end())
    {
        core.dependencies.insert(dependency);
    }
}
std::string decl_func_args(Core& core, std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args)
{
    std::string stringifiedArgs;
    unsigned loops = 0;
    for (auto& arg : args)
    {
        ++loops;
        stringifiedArgs += evaluate_type_stmt(core, std::move(arg.first)) + " " + arg.second;
        if (loops != std::move(args).size())
        {
            stringifiedArgs += ",";
        }
    }
    return stringifiedArgs;
}
std::string decl_func(Core& core, const std::string& return_type, const std::string& name, std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args, const std::string& content)
{
    return return_type + " " + name + "(" + decl_func_args(core, std::move(args)) + "){" + content + "}";
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
    // TODO: add generics
    if (stmt->name == "string")
    {
        req(core, "string");
        return "std::string";
    }
    if (stmt->name == "float")
    {
        return "double";
    }
    if (stmt->name == "int" || stmt->name == "void")
    {
        return stmt->name;
    }
    return "_obn_" + stmt->name;
}
std::string evaluate_func_decl_stmt(Core& core, std::unique_ptr<FuncDeclarationStmt> stmt)
{
    std::string content = "";
    for (auto& statement : std::move(stmt->block)->body)
    {
        content += evaluate_stmt(core, std::move(statement));
    }
    const std::string type = evaluate_type_stmt(core, std::move(stmt->type));
    const std::string args = decl_func_args(core, std::move(stmt->args));
    core.forward_declarations += type + " _obn_" + stmt->identifier + "(" + args + ");";
    return type + " _obn_" + stmt->identifier + "(" + args + "){" + content + "}";
}
std::string evaluate_var_decl_stmt(Core& core, std::unique_ptr<VarDeclarationStmt> stmt)
{
    return decl_var(core, stmt->constant, "_obn_" + stmt->identifier, evaluate_type_stmt(core, std::move(stmt->type)), evaluate_expr(core, std::move(stmt->value)));
}
std::string evaluate_return_stmt(Core& core, std::unique_ptr<ReturnStmt> stmt)
{
    if (!stmt->value)
    {
        return "return;";
    }
    return decl_ret(evaluate_expr(core, std::move(stmt->value)));
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
        stmt.release();
        return evaluate_expr(core, std::move(expr->expression)) + ";";
    }
    else if (auto func = dynamic_cast<FuncDeclarationStmt*>(stmt.get()))
    {
        stmt.release();
        return evaluate_func_decl_stmt(core, std::unique_ptr<FuncDeclarationStmt>(func));
    }
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt.get()))
    {
        stmt.release();
        return evaluate_return_stmt(core, std::unique_ptr<ReturnStmt>(ret));
    }
    else if (auto var = dynamic_cast<VarDeclarationStmt*>(stmt.get()))
    {
        stmt.release();
        return evaluate_var_decl_stmt(core, std::unique_ptr<VarDeclarationStmt>(var)) + ";";
    }
    else if (auto import = dynamic_cast<ImportStmt*>(stmt.get()))
    {
        stmt.release();
        return evaluate_import_stmt(core, std::unique_ptr<ImportStmt>(import));
    }
    stmt.release();
    return "UNIMPLEMENTED";
}
std::string evaluate_expr(Core& core, std::unique_ptr<Expr> expr)
{
    if (auto expr2 = dynamic_cast<BinaryExpr*>(expr.get()))
    {
        expr.release();
        return evaluate_binary_expr(core, expr2);
    }
    if (auto expr2 = dynamic_cast<CallExpr*>(expr.get()))
    {
        expr.release();
        return evaluate_call_expr(core, expr2);
    }
    if (auto expr2 = dynamic_cast<IntegerExpr*>(expr.get()))
    {
        const int value = expr2->value;
        expr.release();
        return std::to_string(value);
    }
    if (auto expr2 = dynamic_cast<FloatExpr*>(expr.get()))
    {
        const double value = expr2->value;
        expr.release();
        return std::to_string(value);
    }
    if (auto expr2 = dynamic_cast<SymbolExpr*>(expr.get()))
    {
        const std::string value = expr2->value;
        expr.release();
        return value;
    }
    if (auto expr2 = dynamic_cast<StringExpr*>(expr.get()))
    {
        const std::string value = expr2->value;
        expr.release();
        return value + "\"";
    }
    if (auto expr2 = dynamic_cast<BooleanExpr*>(expr.get()))
    {
        const bool value = expr2->value;
        expr.release();
        return std::to_string(value);
    }
    expr.release();
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
std::string evaluate_call_expr(Core &core, CallExpr *expr)
{
    std::string args = "";
    unsigned loops = 0;
    for (auto& arg : std::move(expr->args))
    {
        ++loops;
        args += evaluate_expr(core, std::move(arg));
        if (loops != std::move(expr->args).size())
        {
            args += ",";
        }
    }
    return "(_obn_" + expr->name + "(" + args + "))";
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
namespace
{
    std::string evaluate_import_stmt(Core& core, std::unique_ptr<ImportStmt> stmt)
    {
        if (stmt->from.empty())
        {
            error(1, ErrorType::err_invalid_import, "Import path cannot be empty!", stmt->column, stmt->line, stmt->snippet, stmt->fileName);
            return "";
        }
        if (stmt->from[0] == '@')
        {
            const char* stdPath = std::getenv("OBN_STD_PATH");
            const std::filesystem::path headerName = stmt->from.substr(1) + ".h";
            std::filesystem::path filePath = (std::filesystem::path(stdPath ? stdPath : "") / headerName).lexically_normal();
            std::FILE* file = std::fopen(filePath.string().c_str(), "r");
            if (file == NULL)
            {
                std::filesystem::path sourceDirectory = normalizedFilePath(core.file).parent_path();
                while (!sourceDirectory.empty())
                {
                    const std::filesystem::path fallbackPath = sourceDirectory / "src" / "std" / headerName;
                    file = std::fopen(fallbackPath.string().c_str(), "r");
                    if (file != NULL)
                    {
                        filePath = fallbackPath;
                        break;
                    }
                    const std::filesystem::path parent = sourceDirectory.parent_path();
                    if (parent == sourceDirectory)
                    {
                        break;
                    }
                    sourceDirectory = parent;
                }
            }
            if (file == NULL)
            {
                error(1, ErrorType::err_invalid_import, "Tried to import " + stmt->from + "; file does not exist!", stmt->column, stmt->line, stmt->snippet, core.file);
                return "";
            }
            std::fclose(file);
            core.imports += "#include\"" + filePath.string() + "\"\n";
            return "";
        }
        std::filesystem::path requestedPath(stmt->from);
        if (requestedPath.extension().empty())
        {
            requestedPath += ".obn";
        }
        const std::filesystem::path importerPath(stmt->fileName);
        const std::filesystem::path importedPath = normalizedFilePath(requestedPath.is_absolute() ? requestedPath : importerPath.parent_path() / requestedPath);
        const std::string importedFile = importedPath.string();
        const std::string importedKey = importedPath.string();
        std::unique_ptr<std::FILE, decltype(&std::fclose)> importedSourceFile(std::fopen(importedFile.c_str(), "rb"), &std::fclose);
        if (!importedSourceFile)
        {
            error(1, ErrorType::err_invalid_import, "Tried to import " + stmt->from + "; file does not exist!", stmt->column, stmt->line, stmt->snippet, stmt->fileName);
            return "";
        }
        std::string importedSource;
        std::array<char, 8192> buffer;
        std::size_t bytesRead = 0;
        while ((bytesRead = std::fread(buffer.data(), 1, buffer.size(), importedSourceFile.get())) > 0)
        {
            importedSource.append(buffer.data(), bytesRead);
        }
        if (std::ferror(importedSourceFile.get()))
        {
            error(1, ErrorType::err_invalid_import, "Could not read imported file " + stmt->from + ".", stmt->column, stmt->line, stmt->snippet, stmt->fileName);
            return "";
        }
        const std::vector<Token> tokens = tokenise(importedSource, importedFile);
        if (has_errors())
        {
            return "";
        }
        Parser parser(tokens, importedSource, importedFile);
        BlockStmt block = parser.parse();
        if (has_errors())
        {
            return "";
        }
        return generate(core, std::move(block));
    }
}