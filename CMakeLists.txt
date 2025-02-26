cmake_minimum_required(VERSION 3.5)
project(engine VERSION 0.0.1)
option(ENGINE_BUILD_TESTS "Build tests" ON)

# produces compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for CPM
include(cmake/CPM.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
# set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic ")

# for windows
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(common_sources
	src/Engine.h src/Engine.cpp
	src/Position.h
	src/Constants.h
	src/Board.h
	src/Move.h
	src/Pieces.h
	src/Bitboard.h
	src/Utility.h
	src/Move_generator.h src/Move_generator.cpp
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
		GIT_TAG v0.2.0
		DOWNLOAD_ONLY TRUE
	)
	CPMAddPackage(
		NAME Boost
		VERSION 1.84.0
		URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
		URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
		OPTIONS "BOOST_ENABLE_CMAKE ON"
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
	target_link_libraries(tests PRIVATE doctest Boost::process)
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
	add_dependencies(tests build_rust perft)
endif()

add_executable(${PROJECT_NAME} src/main.cpp
	${common_sources}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
