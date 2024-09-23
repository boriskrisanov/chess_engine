cmake_minimum_required(VERSION 3.29)
project(chess_cpp)

set(CMAKE_CXX_STANDARD 23)

add_executable(chess_cpp main.cpp
        board.cpp
        board.hpp
        Piece.hpp
        Move.hpp
        move.cpp
        movegen.cpp
        movegen.hpp
        bitboards.cpp
        bitboards.hpp
        Square.hpp
        utils.hpp
        tests.hpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-fconstexpr-steps=10000000")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/constexpr:steps10000000 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /Oi /arch:AVX2 /GL")
endif ()
