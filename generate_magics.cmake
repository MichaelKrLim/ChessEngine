add_executable(generate_magics utils/magic_main.cpp
	src/Bitboard.h
	src/Magic_util.h
	src/Constants.h
	src/Pieces.h
	src/Position.h
	src/Magic_generation_util.h
)

target_include_directories(generate_magics PRIVATE src)

add_custom_command(
	TARGET generate_magics POST_BUILD
	COMMAND ${CMAKE_BINARY_DIR}/generate_magics
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/magic_squares.h ${CMAKE_SOURCE_DIR}/src
	COMMENT "Generating magics..."
)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/magic_squares.h")
	add_dependencies(tests           generate_magics)
	add_dependencies(${PROJECT_NAME} generate_magics)
	add_dependencies(perft           generate_magics)
endif()
