cmake_minimum_required(VERSION 3.5.0)
project(game VERSION 0.0.1)

# for intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for windows
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(
	main.cpp
)
set_property(TARGET engine PROPERTY CXX_STANDARD 20)
