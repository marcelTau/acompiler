cmake_minimum_required(VERSION 3.22.1)

project(acompiler)

set(CMAKE_CXX_STANDARD 20)

set(TEST_NAME ${PROJECT_NAME}_tests)

set(SOURCES
    src/scanner.cpp
    src/scanner.h

    src/parser.cpp
    src/parser.h

    src/emitter.h

    src/token.cpp
    src/token.h
)


# Include 3rd party libraries
include(FetchContent)
FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG 92d36e82c437f33562bb32096378746c69d96aac)
FetchContent_Declare(googletest GIT_REPOSITORY https://github.com/google/googletest.git GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850)

FetchContent_MakeAvailable(googletest fmt)

# Create the executable
add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME} fmt)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror -pedantic)


# Test configurations
enable_testing()

# Create test executable
add_executable(
    ${TEST_NAME}
    test/scanner.cpp
    test/token.cpp
    test/parser.cpp
    test/emitter.cpp
    ${SOURCES}
)

target_include_directories(${TEST_NAME} PUBLIC src)

target_link_libraries(
    ${TEST_NAME}
    GTest::gtest_main
    fmt
)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
