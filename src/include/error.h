/*
 * Copyright (c) 2026 Obdotgit
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * https://eclipse.org.
 *
 * SPDX-License-Identifier: EPL-2.0
 */
#ifndef ERROR_H
    #define ERROR_H
    #include <cstdio>
    #include <cstdint>
    #include <cstdlib>
    #include <string>
    #include <vector>
    enum ErrorType
    {
        err_unknown_character = 100,
        err_invalid_escape,

        err_unexpected_token = 200,
        err_unexpected_indent
    };
    struct ErrorInfo
    {
        int code;
        std::uint16_t err_no;
        std::string message;
        unsigned col;
        unsigned row;
        std::string snippet;
    };
    void error(int err_code, std::uint16_t err_no, const std::string& message, unsigned col, unsigned row, const std::string& snippet);
    void reset_errors();
    bool has_errors();
    const std::vector<ErrorInfo>& get_errors();
    void print_errors();
#endif