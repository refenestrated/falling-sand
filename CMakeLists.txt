cmake_minimum_required(VERSION 3.10)

project(falling-sand)

add_executable(falling-sand src/main.cpp)

add_library(glad STATIC IMPORTED)
set_target_properties(glad PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libglad.a" INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/glad/")

add_library(SDL3 STATIC IMPORTED)
set_target_properties(SDL3 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libSDL3.a" INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/SDL3/")

if (WIN32)
    target_link_libraries(falling-sand PRIVATE SDL3 glad opengl32)
else()
    target_link_libraries(falling-sand PRIVATE SDL3 glad dl)
endif()

install(TARGETS falling-sand RUNTIME DESTINATION bin)
