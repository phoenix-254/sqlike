cmake_minimum_required(VERSION 3.16)
project(sqlike)

set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_SOURCE_DIR})

find_package(BISON REQUIRED)
BISON_TARGET(Parser Parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

find_package(FLEX REQUIRED)
FLEX_TARGET(Scanner Scanner.l ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)

ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

# a1 project - main
add_executable(sqlike-main-a1 a1-main.cpp Schema.cpp Record.cpp Comparison.cpp ComparisonEngine.cpp
        Page.cpp File.cpp DBFile.cpp ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})

# a1 project - test
add_executable(sqlike-test-a1 a1-test.cpp Schema.cpp Record.cpp Comparison.cpp ComparisonEngine.cpp
        Page.cpp File.cpp BigQ.cpp DBFile.cpp Heap.cpp Sorted.cpp Pipe.cpp
        ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
target_link_libraries(sqlike-test-a1 pthread)

# a2.1 project - test
add_executable(sqlike-test-a21 a21-test.cpp Schema.cpp Record.cpp Comparison.cpp ComparisonEngine.cpp
        Page.cpp File.cpp BigQ.cpp DBFile.cpp Heap.cpp Sorted.cpp Pipe.cpp
        ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
target_link_libraries(sqlike-test-a21 pthread)

# a2.2 project - test
add_executable(sqlike-test test.cpp Schema.cpp Record.cpp Comparison.cpp ComparisonEngine.cpp
        Page.cpp File.cpp BigQ.cpp DBFile.cpp Heap.cpp Sorted.cpp Pipe.cpp
        ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
target_link_libraries(sqlike-test pthread)

find_package(GTest REQUIRED)
add_executable(gtest GTest.cpp Schema.cpp Record.cpp Comparison.cpp ComparisonEngine.cpp
        Page.cpp File.cpp BigQ.cpp DBFile.cpp ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS})
target_link_libraries(gtest ${GTEST_LIBRARIES} pthread)