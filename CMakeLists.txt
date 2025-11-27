cmake_minimum_required(VERSION 3.10)

project(falling-sand)

find_package(SDL3 REQUIRED)

add_executable(falling-sand src/main.cpp src/glad/src/glad.c src/automata/automata.cpp)

target_link_libraries(falling-sand PRIVATE SDL3::SDL3-static)

install(TARGETS falling-sand RUNTIME DESTINATION bin)
