cmake_minimum_required(VERSION 3.23)
project(engine VERSION 0.0.1)

# produces compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for windows
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME} main.cpp
	Engine.h Engine.cpp
	Position.h
	Constants.h
	Board.h
	Move.h
	Pieces.h
	Bitboard.h Bitboard.cpp
	Move_generator.h Move_generator.cpp
)

set_property(TARGET engine PROPERTY CXX_STANDARD 23)
set_property(TARGET engine PROPERTY CXX_EXTENSIONS OFF)
