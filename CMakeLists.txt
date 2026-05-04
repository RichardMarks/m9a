cmake_minimum_required(VERSION 4.2)
project(m9a_dev CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(m9a)
target_sources(m9a
        PRIVATE
        src/utils.cpp
        src/cmd_line.cpp
        src/expression_parser.cpp
        src/read_source.cpp
        src/preprocessor.cpp
        src/m9a.cpp
)

target_include_directories(m9a PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
