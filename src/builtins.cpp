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
namespace
{
    struct PrintBuiltin
    {
        const char* name;
        const char* stream;
        bool newline;
    };
    constexpr PrintBuiltin print_builtins[] = {
        {"print", "stdout", false},
        {"println", "stdout", true},
        {"printError", "stderr", false},
        {"printlnError", "stderr", true}
    };
    constexpr char print_support[] = R"(template<bool Newline, typename... Args>
void print_values(std::FILE* stream, Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        if constexpr (Newline) std::fprintf(stream, "\n");
        return;
    }
    std::size_t i = 0;
    ([&](auto&& value) {
        using D = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<D, std::string>) std::fprintf(stream, "%s", value.c_str());
        else if constexpr (std::is_convertible_v<D, const char*>) std::fprintf(stream, "%s", (const char*)value);
        else if constexpr (requires { value.str(); }) std::fprintf(stream, "%s", value.str().c_str());
        else if constexpr (requires { value.to_string(); }) std::fprintf(stream, "%s", value.to_string().c_str());
        else if constexpr (std::is_same_v<D, bool>) std::fprintf(stream, value ? "true" : "false");
        else if constexpr (std::is_integral_v<D>) std::fprintf(stream, std::is_signed_v<D> ? "%lld" : "%llu", value);
        else if constexpr (std::is_floating_point_v<D>) std::fprintf(stream, "%g", (double)value);
        else if constexpr (std::is_same_v<D, char>) std::fprintf(stream, "%c", value);
        else static_assert(sizeof(D) == 0, "Unsupported type!");
        std::fprintf(stream, ++i < sizeof...(args) ? " " : (Newline ? "\n" : ""));
    }(std::forward<Args>(args)), ...);
})";
}

std::string get_std(Core& core, const std::string& name, const std::string& args)
{
    if (core.builtins_set.find(name) != core.builtins_set.end())
    {
        return "(_obn_" + name + "(" + args + "))";
    }
    core.builtins_set.insert(name);
    for (const PrintBuiltin& builtin : print_builtins)
    {
        if (name != builtin.name)
        {
            continue;
        }
        req(core, "cstdio");
        req(core, "string");
        req(core, "type_traits");
        req(core, "utility");
        if (!core.print_support_emitted)
        {
            core.builtins += print_support;
            core.print_support_emitted = true;
        }
        core.forward_declarations += "template<typename... Args> void _obn_" + name + "(Args&&... args);";
        core.builtins += "template<typename... Args>\nvoid _obn_" + name + "(Args&&... args) {\n    print_values<";
        core.builtins += builtin.newline ? "true" : "false";
        core.builtins += ">(";
        core.builtins += builtin.stream;
        core.builtins += ", std::forward<Args>(args)...);\n}";
        break;
    }
    return "(_obn_" + name + "(" + args + "))";
}