cmake_minimum_required (VERSION 3.6)
project(BT)

add_executable(BT BTbot.cpp)

add_subdirectory(deps/sleepy-discord)
target_link_libraries(BT sleepy-discord)