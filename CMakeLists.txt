cmake_minimum_required(VERSION 3.24)
project(opengl_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8  # Using a stable tag is safer than 'master'
)

FetchContent_MakeAvailable(glm)

# 1. Find Packages
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)

# 2. GLAD Library
# It's better to define GLAD as a static library
add_library(GLAD STATIC glad.c)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/includes ${CMAKE_SOURCE_DIR}/headers)

# 3. Main Executable
add_executable(opengl_project
    main.cpp
    src/shader.cpp
    src/camera.cpp
    src/mesh.cpp
    src/scene.cpp
    src/stb_image.cpp
)

# 4. Link Libraries (The "Modern" Way)
target_link_libraries(opengl_project 
    PRIVATE 
    GLAD 
    glfw 
    OpenGL::GL
    glm::glm
)

# 5. Handle the 'dl' library for Linux only
if(UNIX AND NOT APPLE)
    target_link_libraries(opengl_project PRIVATE dl)
endif()

# 6. Include headers for the main project
target_include_directories(opengl_project PRIVATE 
    ${CMAKE_SOURCE_DIR}/includes 
    ${CMAKE_SOURCE_DIR}/headers
)

# 7. Copy Assets to Build Directory
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
