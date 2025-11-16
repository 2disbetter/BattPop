cmake_minimum_required(VERSION 3.28)
project(BattPop)

set(CMAKE_CXX_STANDARD 11)

add_executable(BattPop main.cpp
        resource.h
        resource.rc)
set_target_properties(BattPop PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(BattPop user32 shell32)
