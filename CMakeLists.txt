cmake_minimum_required(VERSION 3.23)
project(engine VERSION 0.0.1)

# for intellisense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for windows
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(
	engine
	main.cpp
	Engine.cpp Engine.h
	Position.h
	Constants.h
	FEN.h FEN.cpp
	Board.h
	Move.h
	Pieces.h
	Legal_move_generator.h Legal_move_generator.cpp
)

set_property(TARGET engine PROPERTY CXX_STANDARD 23)
set_property(TARGET engine PROPERTY CXX_EXTENSIONS OFF)
