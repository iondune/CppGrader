cmake_minimum_required(VERSION 2.8)

# Name of the project
project(CppGrader)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

target_link_libraries(${CMAKE_PROJECT_NAME} stdc++fs pthread)

include_directories("${PROJECT_SOURCE_DIR}/ext")
