cmake_minimum_required(VERSION 3.20)
project(PgnDatabase)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(pgn_app src/main.cpp src/Parser.cpp src/Database.cpp)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(pgn_app PRIVATE -stdlib=libc++)
    target_link_options(pgn_app PRIVATE -stdlib=libc++)
endif()