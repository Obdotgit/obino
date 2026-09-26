cmake_minimum_required(VERSION 3.15)
project(obino LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
add_executable(obino ${SOURCES})
target_include_directories(obino PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_property(TARGET obino PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_compile_definitions(obino PRIVATE _CRT_SECURE_NO_WARNINGS)

set_target_properties(obino PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../dist")
