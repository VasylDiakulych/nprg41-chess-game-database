cmake_minimum_required(VERSION 3.20)
project(PgnDatabase)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_DEBUG "Enable debug output" OFF)

add_executable(app src/main.cpp src/Parser/Parser.cpp src/Database/Database.cpp src/General/Model.cpp src/Writer/Writer.cpp src/CLI/Cli.cpp)

if(ENABLE_DEBUG)
    target_compile_definitions(app PRIVATE DEBUG)
    target_compile_options(app PRIVATE -g)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(app PRIVATE -stdlib=libc++)
    target_link_options(app PRIVATE -stdlib=libc++)
endif()