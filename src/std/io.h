/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#ifndef STDLIB_IO_H
    #define STDLIB_IO_H
    #include <cstdio>
    #include <string>
    #include <type_traits>
    template <typename...> constexpr bool always_false = false;
    template<bool Newline, typename... Args> void print_values(std::FILE* stream, Args&&... args) {
        if constexpr (sizeof...(args) == 0) {
            if constexpr (Newline) std::fprintf(stream, "\n");
            return;
        }
        std::size_t i = 0;
        ([&](auto&& value) {
            using D = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<D, std::string>) std::fprintf(stream, "%s", value.c_str());
            else if constexpr (std::is_convertible_v<D, const char*>) std::fprintf(stream, "%s", static_cast<const char*>(value));
            else if constexpr (requires { value.str(); }) std::fprintf(stream, "%s", value.str().c_str());
            else if constexpr (requires { value.to_string(); }) std::fprintf(stream, "%s", value.to_string().c_str());
            else if constexpr (std::is_same_v<D, bool>) std::fprintf(stream, value ? "true" : "false");
            else if constexpr (std::is_same_v<D, char>) std::fprintf(stream, "%c", value);
            else if constexpr (std::is_integral_v<D>) {
                if constexpr (std::is_signed_v<D>) std::fprintf(stream, "%lld", static_cast<long long>(value));
                else std::fprintf(stream, "%llu", static_cast<unsigned long long>(value));
            }
            else if constexpr (std::is_floating_point_v<D>) std::fprintf(stream, "%g", static_cast<double>(value));
            else static_assert(always_false<D>, "Unsupported type!");
            std::fprintf(stream, ++i < sizeof...(args) ? " " : (Newline ? "\n" : ""));
        }(std::forward<Args>(args)), ...);
    }
    template<typename... Args> void _obn_print(Args&&... args)
    {
        print_values<false>(stdout, std::forward<Args>(args)...);
    }
    template<typename... Args> void _obn_println(Args&&... args)
    {
        print_values<true>(stdout, std::forward<Args>(args)...);
    }
    template<typename... Args> void _obn_err(Args&&... args)
    {
        print_values<false>(stderr, std::forward<Args>(args)...);
    }
    template<typename... Args> void _obn_errln(Args&&... args)
    {
        print_values<true>(stderr, std::forward<Args>(args)...);
    }
    template<typename... Args> std::string _obn_input(Args&&... args)
    {
        _obn_print(std::forward<Args>(args)...);
        std::string result;
        result.reserve(1024);
        constexpr std::size_t CHUNK_SIZE = 512;
        char buffer[CHUNK_SIZE];
        int ch;
        while ((ch = std::getchar()) != EOF && std::isspace(ch))
        {}
        if (ch == EOF) return result;
        result.push_back(static_cast<char>(ch));
        while ((ch = std::getchar()) != EOF && !std::isspace(ch)) {
            result.push_back(static_cast<char>(ch));
        }
        return result;
    }
#endif