# Obino

## Versions
**1.0.0** - the initial version of Obino. Written from 17 Sep 2026 to 24 Sep 2026 by Obdotgit.

## Compilation
Prerequisites:
* (Windows only) Visual Studio Build Tools for C++ desktop development

Build the executable with:

```sh
npm run build
```

Compile an Obino source file to an executable:

```sh
dist/Release/obino test.obn
dist/Release/obino test.obn -o hello.exe
dist/Release/obino test.obn --output hello.exe
```

The default output is `a.exe` on Windows and `a.out` on other platforms. Set the `CXX` environment variable to choose the compiler used for generated code; otherwise Obino uses `clang++` on Windows and `c++` elsewhere.