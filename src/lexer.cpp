/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "include/lexer.h"
#include "include/error.h"
const std::uint8_t getKindFromString(const std::string& str)
{
    static const std::unordered_map<std::string, TokenKind> keywords = {
        {"if", TokenKind::tok_if},
        {"elseif", TokenKind::tok_elseif},
        {"else", TokenKind::tok_else},
        {"const", TokenKind::tok_const},
        {"int", TokenKind::tok_predefined_type},
        {"float", TokenKind::tok_predefined_type},
        {"string", TokenKind::tok_predefined_type},
        {"for", TokenKind::tok_for},
        {"while", TokenKind::tok_while},
        {"type", TokenKind::tok_type},
        {"public", TokenKind::tok_public},
        {"private", TokenKind::tok_private},
        {"protected", TokenKind::tok_protected},
        {"where", TokenKind::tok_where},
        {"return", TokenKind::tok_return},
        {"void", TokenKind::tok_predefined_type},
        {"boolean", TokenKind::tok_predefined_type},
        {"true", TokenKind::tok_true},
        {"false", TokenKind::tok_false},
        {"import", TokenKind::tok_import}
    };
    auto it = keywords.find(str);
    if (it != keywords.end())
    {
        return it->second;
    }
    return TokenKind::tok_ident;
}
std::uint32_t hexStringToCodePoint(const std::string& hex_str, const std::string& fileName)
{
    std::uint32_t code_point = 0;
    const char* begin = hex_str.data();
    const char* end = begin + hex_str.size();
    auto [ptr, ec] = std::from_chars(begin, end, code_point, 16);

    if (ec != std::errc{} || ptr != end) {
        error(1, ErrorType::err_invalid_escape, "Escape \\u{" + hex_str + "} is invalid hexadecimal!", 1, 1, "\\u{" + hex_str + "}", fileName);
    }

    return code_point;
}
std::string codePointToUTF8(std::uint32_t cp) {
    std::string result;
    if (cp <= 0x7F)
    {
        result.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7FF)
    {
        result.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else if (cp <= 0xFFFF)
    {
        result.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    else if (cp <= 0x10FFFF)
    {
        result.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
    return result;
}
std::string unicodeFromHexString(const std::string& hex_str, const std::string& fileName)
{
    const std::uint32_t cp = hexStringToCodePoint(hex_str, fileName);
    return codePointToUTF8(cp);
}
std::vector<Token> tokenise(const std::string& src, const std::string& fileName)
{
    std::vector<Token> tokens = {};
    std::vector<unsigned> indentStack = { 0 };
    bool atLineStart = true;
    std::size_t pos = 0;
    unsigned row = 1;
    unsigned col = 1;
    const std::size_t srcLength = src.length();
    const auto token = [&row, &col](const std::uint8_t kind, const std::string value) -> Token
    {
        return { kind, value, col, row };
    };
    const auto currentLineSnippet = [&src](const std::size_t startPosition) -> std::string
    {
        const std::size_t lineStart = src.rfind('\n', startPosition) == std::string::npos ? 0 : src.rfind('\n', startPosition) + 1;
        const std::size_t lineEnd = src.find('\n', startPosition) == std::string::npos ? src.size() : src.find('\n', startPosition);
        return src.substr(lineStart, lineEnd - lineStart);
    };
    const auto fail = [&src, &row, &col, &pos, &currentLineSnippet, &fileName](const std::uint16_t err_no, const std::string& message) -> void {
        error(1, err_no, message, col, row, currentLineSnippet(pos), fileName);
    };
    const auto peek = [&pos, &src, srcLength](const std::size_t n = 1) -> std::variant<char, std::nullptr_t>
    {
        if (pos + n >= srcLength) return nullptr;
        return src.at(pos + n);
    };
    const auto variantIsNull = [](std::variant<char, std::nullptr_t> variant) -> std::uint8_t {
        return std::holds_alternative<std::nullptr_t>(variant);
    };
    while (pos < srcLength)
    {
        if (atLineStart)
        {
            unsigned indentation = 0;
            while (pos < srcLength)
            {
                if (src.at(pos) == ' ')
                {
                    ++indentation;
                    ++pos;
                    ++col;
                }
                else if (src.at(pos) == '\t')
                {
                    indentation += 4;
                    ++pos;
                    ++col;
                }
                else
                {
                    break;
                }
            }
            if (pos < srcLength && src.at(pos) != '\n')
            {
                const unsigned currentIndent = indentStack.back();
                if (indentation > currentIndent)
                {
                    indentStack.push_back(indentation);
                    tokens.push_back(token(TokenKind::tok_indent, ""));
                }
                else if (indentation < currentIndent)
                {
                    while (indentStack.size() > 1 && indentation < indentStack.back())
                    {
                        indentStack.pop_back();
                        tokens.push_back(token(TokenKind::tok_dedent, ""));
                    }
                }
            }
            atLineStart = false;
            if (pos >= srcLength) break;
        }
        const char current = src.at(pos);
        switch (current)
        {
            case ' ':
            case '\r':
                ++pos;
                ++col;
                break;
            case '\t':
                ++pos;
                ++col;
                break;
            case '\n':
                tokens.push_back(token(TokenKind::tok_newline, "\n"));
                ++pos;
                ++row;
                col = 1;
                atLineStart = true;
                break;
            case '+':
            {
                const auto next = peek();
                if (!variantIsNull(next)) {
                    const char character = std::get<char>(next);
                    if (character == '+')
                    {
                        tokens.push_back(token(TokenKind::tok_plus_plus, "++"));
                        pos += 2;
                        col += 2;
                        break;
                    }
                    else if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_plus_eq, "+="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_plus, "+"));
                ++pos;
                ++col;
                break;
            }
            case '-':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '-')
                    {
                        tokens.push_back(token(TokenKind::tok_minus_minus, "--"));
                        pos += 2;
                        col += 2;
                        break;
                    }
                    else if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_minus_eq, "-="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_minus, "-"));
                ++pos;
                ++col;
                break;
            }
            case '*':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_star_eq, "*="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_star, "*"));
                ++pos;
                ++col;
                break;
            }
            case '/':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_slash_eq, "/="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                    else if (character == '/')
                    {
                        pos += 2;
                        col += 2;
                        while (pos < src.length() && src.at(pos) != '\n')
                        {
                            ++pos;
                            ++col;
                        }
                        break;
                    }
                    else if (character == '*')
                    {
                        pos += 2;
                        col += 2;
                        bool terminated = false;
                        while (pos < srcLength)
                        {
                            if (src.at(pos) == '*' && pos + 1 < srcLength && src.at(pos + 1) == '/')
                            {
                                pos += 2;
                                col += 2;
                                terminated = true;
                                break;
                            }
                            if (src.at(pos) == '\n')
                            {
                                ++pos;
                                ++row;
                                col = 1;
                                atLineStart = true;
                            }
                            else
                            {
                                ++pos;
                                ++col;
                            }
                        }
                        if (!terminated)
                        {
                            fail(err_unknown_character, "Unterminated block comment.");
                        }
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_slash, "/"));
                ++pos;
                ++col;
                break;
            }
            case '%':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_percent_eq, "%="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_percent, "%"));
                ++pos;
                ++col;
                break;
            }
            case '#':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_hash_eq, "#="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_hash, "#"));
                ++pos;
                ++col;
                break;
            }
            case '^':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_caret_eq, "^="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_caret, "^"));
                ++pos;
                ++col;
                break;
            }
            case '>':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_gt_eq, ">="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_gt, ">"));
                ++pos;
                ++col;
                break;
            }
            case '<':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_lt_eq, "<="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_lt, "<"));
                ++pos;
                ++col;
                break;
            }
            case '!':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_not_eq, "!="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_not, "!"));
                ++pos;
                ++col;
                break;
            }
            case '=':
            {
                const auto next = peek();
                if (!variantIsNull(next))
                {
                    const char character = std::get<char>(next);
                    if (character == '=')
                    {
                        tokens.push_back(token(TokenKind::tok_eq_eq, "=="));
                        pos += 2;
                        col += 2;
                        break;
                    }
                }
                tokens.push_back(token(TokenKind::tok_eq, "="));
                ++pos;
                ++col;
                break;
            }
            case '&':
                tokens.push_back(token(TokenKind::tok_and, "&"));
                ++pos;
                ++col;
                break;
            case '|':
                tokens.push_back(token(TokenKind::tok_or, "|"));
                ++pos;
                ++col;
                break;
            case ',':
                tokens.push_back(token(TokenKind::tok_comma, ","));
                ++pos;
                ++col;
                break;
            case '.':
                tokens.push_back(token(TokenKind::tok_dot, "."));
                ++pos;
                ++col;
                break;
            case ':':
                tokens.push_back(token(TokenKind::tok_colon, ":"));
                ++pos;
                ++col;
                break;
            case '(':
                tokens.push_back(token(TokenKind::tok_open_paren, "("));
                ++pos;
                ++col;
                break;
            case ')':
                tokens.push_back(token(TokenKind::tok_closed_paren, ")"));
                ++pos;
                ++col;
                break;
            case '[':
                tokens.push_back(token(TokenKind::tok_open_bracket, "["));
                ++pos;
                ++col;
                break;
            case ']':
                tokens.push_back(token(TokenKind::tok_closed_bracket, "]"));
                ++pos;
                ++col;
                break;
            default:
            {
                if (std::isalpha(static_cast<unsigned char>(current)))
                {
                    std::string ident(1, current);
                    ++pos;
                    ++col;
                    while (pos < srcLength && (std::isalnum(static_cast<unsigned char>(src.at(pos))) || src.at(pos) == '_'))
                    {
                        ident += src.at(pos);
                        ++pos;
                        ++col;
                    }
                    tokens.push_back(token(getKindFromString(ident), ident));
                }
                else if (std::isdigit(static_cast<unsigned char>(current)))
                {
                    std::string num(1, current);
                    std::uint8_t dotCount = 0;
                    ++pos;
                    ++col;
                    while (pos < srcLength && (std::isdigit(static_cast<unsigned char>(src.at(pos))) || src.at(pos) == '.'))
                    {
                        if (src.at(pos) == '.')
                        {
                            if (dotCount == 1)
                            {
                                fail(err_unknown_character, "Malformed numeric literal '" + num + "'.");
                            }
                            ++dotCount;
                        }
                        num += src.at(pos);
                        ++pos;
                        ++col;
                    }
                    tokens.push_back(token(dotCount ? TokenKind::tok_float : TokenKind::tok_int, num));
                }
                else if (current == '\'' || current == '"')
                {
                    const char startingCharacter = current;
                    std::string str(1, startingCharacter);
                    ++pos;
                    ++col;
                    while (pos < srcLength && src.at(pos) != startingCharacter)
                    {
                        if (src.at(pos) == '\\')
                        {
                            ++pos;
                            ++col;
                            if (pos >= srcLength)
                            {
                                fail(err_invalid_escape, "Unterminated escape sequence in string literal.");
                            }
                            switch (src.at(pos))
                            {
                                case 'n':
                                    str += '\n';
                                    break;
                                case 't':
                                    str += '\t';
                                    break;
                                case 'u':
                                {
                                    ++pos;
                                    ++col;
                                    if (pos >= srcLength || src.at(pos) != '{')
                                    {
                                        fail(err_invalid_escape, "Unicode escape sequences must use the form \\u{...}.");
                                    }
                                    std::string code;
                                    while (pos < srcLength && src.at(pos) != '}')
                                    {
                                        code += src.at(pos);
                                        ++pos;
                                        ++col;
                                    }
                                    if (pos >= srcLength || src.at(pos) != '}')
                                    {
                                        fail(err_invalid_escape, "Unterminated unicode escape sequence in string literal.");
                                    }
                                    str += unicodeFromHexString(code, fileName);
                                    ++pos;
                                    ++col;
                                    break;
                                }
                                default:
                                    str += src.at(pos);
                                    break;
                            }
                        }
                        else
                        {
                            str += src.at(pos);
                        }
                        ++pos;
                        ++col;
                    }
                    if (pos >= srcLength)
                    {
                        fail(err_unknown_character, "Unterminated string literal.");
                    }
                    ++pos;
                    ++col;
                    tokens.push_back(token(TokenKind::tok_string, str));
                }
                else
                {
                    fail(err_unknown_character, "Unexpected character '" + std::string(1, current) + "'.");
                    ++pos;
                    ++col;
                }
                break;
            }
        }
    }
    while (indentStack.size() > 1)
    {
        indentStack.pop_back();
        tokens.push_back(token(TokenKind::tok_dedent, ""));
    }
    tokens.push_back(token(TokenKind::tok_eof, ""));
    return tokens;
}