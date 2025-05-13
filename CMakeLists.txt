cmake_minimum_required(VERSION 3.7)
project(engine VERSION 0.0.1)
option(ENGINE_BUILD_TESTS "Build tests" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for CPM
include(cmake/CPM.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic")

# for windows
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(common_sources
	src/Engine.h src/Engine.cpp
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
)

add_executable(generate_magics src/magic_main.cpp
	src/Bitboard.h
	src/Magic_util.h
	src/Constants.h
	src/Pieces.h
	src/Position.h
)

set_property(TARGET generate_magics PROPERTY CXX_STANDARD 23)
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
		GIT_TAG v2.4.11
	)
	CPMAddPackage(
		NAME perftree
		GIT_REPOSITORY https://github.com/agausmann/perftree.git
		GIT_TAG 0f5bc12
		DOWNLOAD_ONLY TRUE
	)
	file(GLOB test_sources CONFIGURE_DEPENDS src/*.tests.cpp)
	add_executable(tests
		${test_sources}
		${common_sources}
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
	set_property(TARGET tests PROPERTY CXX_STANDARD 23)
	set_property(TARGET tests PROPERTY CXX_EXTENSIONS OFF)
	set_property(TARGET perft PROPERTY CXX_STANDARD 23)
	set_property(TARGET perft PROPERTY CXX_EXTENSIONS OFF)
	# get perftree
	set(RUST_PACKAGE_DIRS "${CMAKE_BINARY_DIR}/_deps")
	add_custom_target(
		build_rust
		COMMAND cargo build --bin perftree --release --manifest-path=${RUST_PACKAGE_DIRS}/perftree-src/Cargo.toml
		WORKING_DIRECTORY ${RUST_PACKAGE_DIRS}/perftree-build
		COMMENT "Building perftree..."
	)
	add_custom_command(
		TARGET build_rust POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${RUST_PACKAGE_DIRS}/perftree-src/target/release/perftree ${CMAKE_BINARY_DIR}
		COMMENT "Copying Rust executable to project directory"
	)
	add_dependencies(tests build_rust perft generate_magics)
endif()

add_executable(${PROJECT_NAME} src/main.cpp
	${common_sources}
	src/Uci_handler.cpp src/Uci_handler.h
)

if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/magic_squares.h")
	add_dependencies(engine generate_magics)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
