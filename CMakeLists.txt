cmake_minimum_required(VERSION 3.23)
project(engine VERSION 0.0.1)
option(ENGINE_BUILD_TESTS "Build tests" ON)

# produces compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for CPM
include(cmake/CPM.cmake)

# for windows
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#for doctest
CPMAddPackage(
    NAME doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)

set(common_sources
	src/Engine.h src/Engine.cpp
	src/Position.h
	src/Constants.h
	src/Board.h
	src/Move.h
	src/Pieces.h
	src/Bitboard.h src/Bitboard.cpp
	# src/Move_generator.h src/Move_generator.cpp
)

if(ENGINE_BUILD_TESTS)
    file(GLOB test_sources CONFIGURE_DEPENDS src/*.tests.cpp)
    add_executable(tests
        ${test_sources}
        ${common_sources}
        src/test_main.cpp
    )
    target_link_libraries(tests PRIVATE doctest)
    target_compile_definitions(tests PRIVATE DOCTEST_CONFIG_USE_STD_HEADERS)
    if(MSVC)
        target_compile_options(tests PRIVATE /W4)
    else()
        target_compile_options(tests PRIVATE -Wall -pedantic -Werror)
    endif()
    set_property(TARGET tests PROPERTY CXX_STANDARD 23)
    set_property(TARGET tests PROPERTY CXX_EXTENSIONS OFF)
endif()

add_executable(${PROJECT_NAME} src/main.cpp
	${common_sources}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)
