/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#include "include/error.h"
namespace
{
    constexpr const char* RED   = "\033[1;31m";
    constexpr const char* RESET = "\033[0m";
    constexpr const char* BOLD  = "\033[1m";
    std::size_t digits(unsigned value)
    {
        std::size_t count = 1;
        while (value >= 10)
        {
            value /= 10;
            ++count;
        }
        return count;
    }
    std::vector<ErrorInfo>& global_errors()
    {
        static std::vector<ErrorInfo> errors;
        return errors;
    }
}
std::size_t visual_column(const std::string& snippet, unsigned source_col)
{
    constexpr std::size_t TAB_WIDTH = 4;
    std::size_t visual_col = 0;
    unsigned source_col_current = 1;
    for (char c : snippet)
    {
        if (source_col_current >= source_col) break;
        if (c == '\t')
        {
            visual_col += TAB_WIDTH - (visual_col % TAB_WIDTH);
        }
        else
        {
            ++visual_col;
        }
        ++source_col_current;
    }
    return visual_col;
}
std::string expand_tabs(const std::string& str)
{
    constexpr std::size_t TAB_WIDTH = 4;
    std::string result;
    std::size_t visual_col = 0;
    for (char c : str)
    {
        if (c == '\t')
        {
            const std::size_t spaces = TAB_WIDTH - (visual_col % TAB_WIDTH);
            result.append(spaces, ' ');
            visual_col += spaces;
        }
        else
        {
            result += c;
            ++visual_col;
        }
    }
    return result;
}
void error(int err_code, std::uint16_t err_no, const std::string& message, unsigned col, unsigned row, const std::string& snippet, const std::string& fileName)
{
    global_errors().push_back({
        err_code,
        err_no,
        message,
        col,
        row,
        snippet,
        fileName
    });
}
void reset_errors()
{
    global_errors().clear();
}
bool has_errors()
{
    return !global_errors().empty();
}
const std::vector<ErrorInfo>& get_errors()
{
    return global_errors();
}
void print_errors()
{
    for (const auto& err : global_errors())
    {
        const std::size_t gutter = digits(err.row);
        const std::size_t caret_col = visual_column(err.snippet, err.col);
        const std::string display_snippet = expand_tabs(err.snippet);
        std::fprintf(stderr, "%s%serror[OBN-%u]%s: %s\n", RED, BOLD, err.err_no, RESET, err.message.c_str());
        if (err.fileName.empty())
        {
            std::fprintf(stderr, "  --> line %u:%u\n", err.row, err.col);
        }
        else
        {
            std::fprintf(stderr, "  --> %s:%u:%u\n", err.fileName.c_str(), err.row, err.col);
        }
        std::fprintf(stderr, "%*s|\n", static_cast<int>(gutter + 2), "");
        std::fprintf(stderr, " %u | %s\n", err.row, err.snippet.c_str());
        std::fprintf(stderr, "%*s| ", static_cast<int>(gutter + 2), "");
        if (caret_col > 0)
        {
            std::fputs(std::string(caret_col, ' ').c_str(), stderr);
        }
        std::fprintf(stderr, "%s^%s\n", RED, RESET);
    }
}