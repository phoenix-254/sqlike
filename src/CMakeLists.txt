cmake_minimum_required(VERSION 3.16)
project(sqlike)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_SOURCE_DIR})

find_package(BISON REQUIRED)

BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

find_package(FLEX REQUIRED)

FLEX_TARGET(Scanner Scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)

ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

add_executable(sqlike-main
        main.cpp
        Schema.cpp
        Record.cpp
        Comparison.cpp
        ComparisonEngine.cpp
        Page.cpp
        File.cpp
        DBFile.cpp
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Scanner_OUTPUTS}
        )

add_executable(sqlike-test
        test.cpp
        Schema.cpp
        Record.cpp
        Comparison.cpp
        ComparisonEngine.cpp
        Page.cpp
        File.cpp
        DBFile.cpp
        ${BISON_Parser_OUTPUTS}
        ${FLEX_Scanner_OUTPUTS}
        )