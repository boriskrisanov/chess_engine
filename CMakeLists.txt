cmake_minimum_required(VERSION 3.29)
project(chess_cpp)

set(CMAKE_CXX_STANDARD 23)

# BUILD_MODE: "executable" - build exe, "library" - build static lib

if (NOT DEFINED BUILD_MODE)
    message(WARNING "BUILD_MODE is not defined, building executable by default")
    set(BUILD_MODE "executable")
endif ()

if (BUILD_MODE STREQUAL "library")
    add_library(chess_cpp
            src/Board.cpp
            src/Board.hpp
            src/Piece.hpp
            src/Move.hpp
            src/Move.cpp
            src/movegen.cpp
            src/movegen.hpp
            src/bitboards.cpp
            src/bitboards.hpp
            src/Square.hpp
            src/utils.hpp
            src/search.cpp
            src/search.hpp
            src/eval.cpp
            src/eval.hpp
    )
elseif (BUILD_MODE STREQUAL "executable")
    add_executable(chess_cpp src/main.cpp
            src/Board.cpp
            src/Board.hpp
            src/Piece.hpp
            src/Move.hpp
            src/Move.cpp
            src/movegen.cpp
            src/movegen.hpp
            src/bitboards.cpp
            src/bitboards.hpp
            src/Square.hpp
            src/utils.hpp
            src/tests.hpp
            src/tests.cpp   
            src/search.cpp
            src/search.hpp
            src/eval.cpp
            src/eval.hpp
            src/MoveFlag.hpp
            src/MoveList.hpp
            src/magic_searcher.hpp
            src/magic_searcher.cpp
    )
else ()
    message(FATAL_ERROR "BUILD_MODE is invalid, see CMakeLists.txt for valid options")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "-march=native -fconstexpr-steps=10000000")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-march=native -fconstexpr-depth=10000000 -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/constexpr:steps10000000 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /Oi /arch:AVX2 /GL")
endif ()
