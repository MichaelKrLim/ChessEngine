cmake_minimum_required(VERSION 3.7)
project(engine VERSION 0.0.1)
option(ENGINE_BUILD_TESTS "Build tests" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for CPM
include(cmake/CPM.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -funroll-loops -pedantic -O3") # -Werror

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Werror) # prevent gcc bug failing compilation
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# for windows
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(common_sources
	src/Engine.h
	src/Position.h
	src/Constants.h
	src/Move.h
	src/Pieces.h
	src/Bitboard.h
	src/Utility.h
	src/Magic_util.h
	src/State.cpp src/State.h
	src/Move_generator.h src/Move_generator.cpp
	src/Transposition_table.h
	src/bench.h
)

add_executable(generate_magics src/magic_main.cpp
	src/Bitboard.h
	src/Magic_util.h
	src/Constants.h
	src/Pieces.h
	src/Position.h
	src/Magic_generation_util.h
)

set_property(TARGET generate_magics PROPERTY CXX_EXTENSIONS OFF)

add_custom_command(
	TARGET generate_magics POST_BUILD
	COMMAND ${CMAKE_BINARY_DIR}/generate_magics
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/magic_squares.h ${CMAKE_SOURCE_DIR}/src
	COMMENT "Generating magics..."
)

if(ENGINE_BUILD_TESTS)
	CPMAddPackage(
		NAME doctest
		GIT_REPOSITORY https://github.com/doctest/doctest.git
		GIT_TAG v2.4.12
		SYSTEM ON
	)
	file(GLOB test_sources CONFIGURE_DEPENDS src/*.tests.cpp)
	add_executable(tests
		${test_sources}
		${common_sources}
		src/Magic_generation_util.h
		src/test_main.cpp
	)
	add_executable(perft
		${common_sources}
		src/perft_main.cpp
	)
	target_compile_definitions(tests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)
	if(MSVC)
		target_compile_options(tests PRIVATE /W4)
	endif()
	set_property(TARGET tests PROPERTY CXX_EXTENSIONS OFF)
	set_property(TARGET perft PROPERTY CXX_EXTENSIONS OFF)
	if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/magic_squares.h")
		add_dependencies(tests perft generate_magics)
	endif()
	target_link_libraries(tests PRIVATE doctest)
endif()

add_executable(${PROJECT_NAME} src/main.cpp
	${common_sources}
	src/Uci_handler.cpp src/Uci_handler.h
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/magic_squares.h")
	add_dependencies(engine generate_magics)
endif()
