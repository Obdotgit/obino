# Obino

<p align="center">
  <img src="https://static.wikitide.net/obinowiki/b/bb/Obino.png" alt="Obino logo" width="200"/>
</p>

Obino is a compiled, high-level, general-purpose programming language. The core compiler is written in C++ and transpiles source code directly into optimized C++, which then automatically invokes a host C++ compiler to compile itself into a native executable.

---

## Getting Started

### Prerequisites
To build and run Obino, ensure you have the following installed:
* A modern **C++ compiler** supporting C++20 or later (`GCC`, `Clang`, or `MSVC`)
* **CMake** (v3.15+)
* *(Windows Only)* **Visual Studio Build Tools** with the "Desktop development with C++" workload active

### Building the Compiler
Run the bootstrap build script from the project root:

```sh
python build.py
```
*This will automatically generate a release-optimised build and place the executable inside the `dist/` directory.*

---

## Usage

Compile an Obino source file (`.obn`) into a native executable using the following syntax:

```sh
# Compiles to the default target (a.exe or a.out)
dist/Release/obino examples/test.obn

# Compiles to a custom output name using short or long flags
dist/Release/obino examples/test.obn -o hello.exe
dist/Release/obino examples/test.obn --output hello.exe
```

### Compiler Configurations
By default, Obino invokes `clang++` on Windows and `c++` on Unix-like platforms to compile its transpiled C++ output. You can explicitly override this fallback behaviour by setting the `CXX` environment variable:

```sh
# Example: Force Obino to use g++ for code generation
export CXX=g++
dist/Release/obino examples/test.obn
```

| Platform | Default Compiler | Default Output Binary |
| :--- | :--- | :--- |
| **Windows** | `clang++` | `a.exe` |
| **Linux / macOS** | `c++` | `a.out` |

---

## Version History

*   **`1.0.0`** (Released: TBR)
    *   Initial production release of the Obino language core.
    *   Features a transpilation pipeline that outputs C++ source files which handle their own automated compilation.
    *   Authored and architected exclusively by **Obdotgit**.

---

## License
This project is licensed under the terms found in the [LICENSE](LICENSE) file.