#include "include/codegen.h"
#include "include/error.h"
#include "include/lexer.h"
#include "include/parser.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
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
    const std::filesystem::path temporarySource = std::filesystem::temp_directory_path() / "obino-generated.cpp";
    std::ofstream sourceFile(temporarySource, std::ios::binary);
    if (!sourceFile)
    {
        std::fprintf(stderr, "Could not create temporary generated source file.\n");
        return 1;
    }
    std::printf(generated.c_str());
    sourceFile << core.get_result() << "\nint main(){";
    sourceFile << generated;
    sourceFile << "return 0;}\n";
    sourceFile.close();
    const std::string command = compiler_command() + " -std=c++20 -O2 -o " + shell_quote(outputPath) + " " + shell_quote(temporarySource.string());
    const int result = std::system(command.c_str());
    std::error_code cleanupError;
    std::filesystem::remove(temporarySource, cleanupError);
    if (result != 0)
    {
        std::fprintf(stderr, "Compilation failed.\n");
        return 1;
    }
    return 0;
}