/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#ifndef CODEGEN_H
    #define CODEGEN_H
    #include "parser.h"
    #include <array>
    #include <cstdio>
    #include <cstdlib>
    #include <filesystem>
    #include <memory>
    #include <string>
    #include <system_error>
    #include <unordered_set>
    #include <utility>
    #include <vector>
    struct Core
    {
        std::unordered_set<std::string> dependencies;
        std::string forward_declarations;
        std::string imports;
        const std::string file;
        Core(std::string file) : dependencies({}), forward_declarations(""), imports(""), file(file)
        {}
        std::string get_result() const;
    };
    void req(Core& core, const std::string& dependency);
    std::string decl_func(Core& core, const std::string& return_type, const std::string& name, std::vector<std::pair<std::unique_ptr<TypeStmt>, std::string>> args, const std::string& content);
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
    std::string evaluate_func_decl_stmt(Core& core, std::unique_ptr<FuncDeclarationStmt> stmt);
    std::string evaluate_return_stmt(Core& core, std::unique_ptr<ReturnStmt> stmt);
    std::string evaluate_expr(Core& core, std::unique_ptr<Expr> expr);
    std::string evaluate_binary_expr(Core& core, BinaryExpr* expr);
    std::string evaluate_call_expr(Core& core, CallExpr* expr);
#endif