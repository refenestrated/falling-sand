cmake_minimum_required(VERSION 3.10)

project(falling-sand)

add_executable(falling-sand main.cpp)

install(TARGETS falling-sand RUNTIME DESTINATION bin)
