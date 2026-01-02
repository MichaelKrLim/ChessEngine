cmake_minimum_required(VERSION 3.7)
project(ChessEngine VERSION 0.0.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for CPM
include(cmake/CPM.cmake)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall
					-pedantic
					-O3
					-march=native) # -Werror causes bug in gcc

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
	src/bench.h
	src/search.h src/search.cpp
	src/Time_manager.cpp src/Time_manager.h
	src/move_unmove.cpp src/move_unmove.h
	src/nnue/NNUE_header.cpp src/nnue/NNUE_header.h
	src/nnue/Neural_network.cpp src/nnue/Neural_network.h
	src/nnue/common.h
	src/nnue/layers/Dense_linear_layer.h
	src/nnue/layers/Feature_transformer.cpp src/nnue/layers/Feature_transformer.h
	src/nnue/layers/mdspan.h
	src/nnue/Accumulator.h
)


add_executable(perft
	${common_sources}
	utils/perft_main.cpp
)

target_include_directories(perft PRIVATE src)

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

target_compile_definitions(tests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)
if(MSVC)
	target_compile_options(tests PRIVATE /W4)
endif()
target_link_libraries(tests PRIVATE doctest)

add_executable(${PROJECT_NAME} src/main.cpp
	${common_sources}
	src/Uci_handler.h src/Uci_handler.cpp
)

include(generate_magics.cmake)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET tests PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET perft PROPERTY CXX_EXTENSIONS OFF)
set_property(TARGET generate_magics PROPERTY CXX_EXTENSIONS OFF)
