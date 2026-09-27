/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#ifndef LEXER_H
    #define LEXER_H
    #include <cctype>
    #include <charconv>
    #include <cstdint>
    #include <string>
    #include <unordered_map>
    #include <variant>
    #include <vector>
    enum TokenKind
    {
        tok_eof,
        tok_ident,
        tok_int, tok_float, tok_string,
        tok_plus, tok_minus, tok_star, tok_slash, tok_percent, tok_hash, tok_caret,
        tok_plus_plus, tok_minus_minus,
        tok_plus_eq, tok_minus_eq, tok_star_eq, tok_slash_eq, tok_percent_eq, tok_hash_eq, tok_caret_eq,
        tok_eq, tok_const, tok_predefined_type,
        tok_lt, tok_gt, tok_eq_eq, tok_lt_eq, tok_gt_eq, tok_not, tok_not_eq, tok_and, tok_or,
        tok_indent, tok_dedent, tok_newline,
        tok_if, tok_elseif, tok_else,
        tok_for, tok_while,
        tok_type, tok_public, tok_private, tok_protected, tok_where,
        tok_return,
        tok_open_paren, tok_closed_paren,
        tok_open_bracket, tok_closed_bracket,
        tok_comma, tok_dot, tok_colon,
        tok_true, tok_false
    };
    struct Token
    {
        const std::uint8_t kind;
        const std::string value;
        const unsigned col;
        const unsigned row;
    };
    std::vector<Token> tokenise(const std::string& src);
#endif