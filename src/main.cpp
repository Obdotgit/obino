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
#include "include/parser.h"
#include <cstdio>
#include <cstdlib>
#include <string>
namespace
{
    std::string shell_quote(const std::string& value)
    {
        std::string quoted = "\"";
        for (const char character : value)
        {
            if (character == '"')
            {
                quoted += '\\';
            }
            quoted += character;
        }
        return quoted + "\"";
    }
    std::string default_output_path()
    {
        #ifdef _WIN32
            return "a.exe";
        #else
            return "a.out";
        #endif
    }
    std::string compiler_command()
    {
        if (const char* compiler = std::getenv("CXX"))
        {
            return compiler;
        }
        #ifdef _WIN32
            return "clang++";
        #else
            return "c++";
        #endif
    }
}
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::fprintf(stderr, "Usage: %s <source-file> [-o <output>]\n", argv[0]);
        std::fprintf(stderr, "       %s <source-file> [--output <output>]\n", argv[0]);
        return 1;
    }
    std::string outputPath = default_output_path();
    for (int argument = 2; argument < argc; ++argument)
    {
        const std::string option = argv[argument];
        if (option == "-o" || option == "--output")
        {
            if (argument + 1 >= argc)
            {
                std::fprintf(stderr, "%s requires an output path.\n", option.c_str());
                return 1;
            }
            outputPath = argv[++argument];
        }
        else
        {
            std::fprintf(stderr, "Unknown option: %s\n", option.c_str());
            return 1;
        }
    }
    std::FILE* const inputFile = std::fopen(argv[1], "rb");
    if (!inputFile)
    {
        std::fprintf(stderr, "Could not open file %s!\n", argv[1]);
        return 1;
    }
    std::fseek(inputFile, 0, SEEK_END);
    const long fileSize = std::ftell(inputFile);
    std::rewind(inputFile);
    std::string source;
    if (fileSize > 0)
    {
        source.resize(static_cast<std::size_t>(fileSize));
        const std::size_t bytesRead = std::fread(&source[0], 1, source.size(), inputFile);
        source.resize(bytesRead);
    }
    std::fclose(inputFile);
    reset_errors();
    const auto tokens = tokenise(source);
    if (has_errors())
    {
        print_errors();
        return 1;
    }
    Parser parser(tokens, source);
    BlockStmt program = parser.parse();
    if (has_errors())
    {
        print_errors();
        return 1;
    }
    Core core;
    const std::string generated = generate(core, std::move(program));
    char temporarySource[L_tmpnam];
    if (!std::tmpnam(temporarySource))
    {
        std::fprintf(stderr, "Could not create temporary generated source path.\n");
        return 1;
    }
    const std::string temporarySourcePath = std::string(temporarySource) + ".cpp";
    const std::string generatedSource = core.get_result() + "\n" + generated + "int main(){_obn_main();return 0;}";
    std::FILE* const sourceFile = std::fopen(temporarySourcePath.c_str(), "wbx");
    if (!sourceFile)
    {
        std::fprintf(stderr, "Could not create temporary generated source file.\n");
        return 1;
    }
    const std::size_t bytesWritten = std::fwrite(generatedSource.data(), 1, generatedSource.size(), sourceFile);
    const int closeResult = std::fclose(sourceFile);
    if (bytesWritten != generatedSource.size() || closeResult != 0)
    {
        std::fprintf(stderr, "Could not write temporary generated source file.\n");
        std::remove(temporarySourcePath.c_str());
        return 1;
    }
    std::fwrite(generatedSource.data(), 1, generatedSource.size(), stdout);
    const std::string command = compiler_command() + " -std=c++20 -Os -o " + shell_quote(outputPath) + " " + shell_quote(temporarySourcePath);
    const int result = std::system(command.c_str());
    std::remove(temporarySourcePath.c_str());
    if (result != 0)
    {
        std::fprintf(stderr, "Compilation failed.\n");
        return 1;
    }
    return 0;
}