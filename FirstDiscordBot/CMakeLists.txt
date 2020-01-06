cmake_minimum_required (VERSION 3.6)
project(LeOkayHandBot)

add_executable(LeOkayHandBot LeOkayHandBot.cpp)

add_subdirectory(deps/sleepy-discord)
target_link_libraries(LeOkayHandBot sleepy-discord)