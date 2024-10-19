cmake_minimum_required(VERSION 3.29)
project(chess_cpp)

set(CMAKE_CXX_STANDARD 23)

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
        src/search.cpp
        src/search.hpp
        src/eval.cpp
        src/eval.hpp
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-fconstexpr-steps=10000000")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/constexpr:steps10000000 /EHsc")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3 /Oi /arch:AVX2 /GL")
endif ()
