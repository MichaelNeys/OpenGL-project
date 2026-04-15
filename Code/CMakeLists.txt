cmake_minimum_required(VERSION 3.24)
project(opengl_project)

set(CMAKE_CXX_STANDARD 17)

link_directories(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(OpenGL REQUIRED)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

# Vervang "opengl32" door de generieke variabele die CMake zelf vindt
# "glfw" (of glfw3) verwijst naar je pacman installatie
# "dl" is nodig voor GLAD op Linux
set(LIBS glfw ${OPENGL_LIBRARIES} dl)

add_library(GLAD "glad.c")
set(LIBS ${LIBS} GLAD)

include_directories(
    ${CMAKE_SOURCE_DIR}/includes
    ${CMAKE_SOURCE_DIR}/headers
)

file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
file(COPY textures DESTINATION ${CMAKE_BINARY_DIR})

add_executable(opengl_project
    main.cpp
    src/shader.cpp
    src/camera.cpp
    src/mesh.cpp
    src/scene.cpp
    src/stb_image.cpp
)

# Kopieer shaders naar build directory
file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})
target_link_libraries(opengl_project ${LIBS})