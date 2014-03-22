project(Redbase)

cmake_minimum_required(VERSION 2.8)

file(GLOB_RECURSE Redbase_SOURCES "src/*.cpp")

add_executable(redbase ${Redbase_SOURCES})

include_directories("include/")
