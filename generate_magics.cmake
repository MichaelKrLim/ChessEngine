add_executable(generate_magics utils/magic_main.cpp
	src/Bitboard.h
	src/Magic_util.h
	src/Constants.h
	src/Pieces.h
	src/Position.h
	src/Magic_generation_util.h
)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/magic_squares.h")
	execute_process(
		COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target generate_magics
		COMMAND ${CMAKE_BINARY_DIR}/generate_magics
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/magic_squares.h ${CMAKE_SOURCE_DIR}/src
	)
endif()
