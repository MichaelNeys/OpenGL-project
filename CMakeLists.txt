cmake_minimum_required(VERSION 3.30)
project(opengl_project)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# --- 1. Fetch GLM ---
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# --- 2. Fetch GLFW ---
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
# Disable building GLFW tests and docs to save time
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# --- 3. Fetch Assimp ---
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_MINIZIP ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v5.4.3
)
FetchContent_MakeAvailable(assimp)
if(TARGET assimp)
  target_include_directories(assimp PRIVATE
    ${assimp_SOURCE_DIR}/contrib/zlib/contrib/minizip
  )
  target_sources(assimp PRIVATE
    ${assimp_SOURCE_DIR}/contrib/zlib/contrib/minizip/ioapi.c
    ${assimp_SOURCE_DIR}/contrib/zlib/contrib/minizip/unzip.c
  )
endif()

# --- 4. Find System Packages ---
find_package(OpenGL REQUIRED)

# --- 5. GLAD Library ---
add_library(GLAD STATIC glad.c)
target_include_directories(GLAD PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_link_libraries(GLAD PRIVATE OpenGL::GL)

# --- 6. Main Executable ---
add_executable(opengl_project
    main.cpp
    src/InputManager.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Scene.cpp
    src/Shader.cpp
    src/stb_image.cpp
    src/Terrain.cpp
    src/Skybox.cpp
    src/PostProcess.cpp
    src/Bloom.cpp
    src/ChromaKey.cpp
    src/Track/BezierCurve.cpp
    src/Track/BezierPath.cpp
    src/Track/TrackRenderer.cpp
    src/Lighting/LightManager.cpp
)

# --- 7. Link Libraries ---
target_link_libraries(opengl_project 
    PRIVATE 
    GLAD 
    glfw
    OpenGL::GL
    glm::glm
    assimp::assimp
)

# 7. Handle the 'dl' library for Linux only
if(UNIX AND NOT APPLE)
    target_link_libraries(opengl_project PRIVATE dl)
endif()

# 8. Include headers for the main project
target_include_directories(opengl_project PRIVATE 
    ${CMAKE_SOURCE_DIR}/headers
)

# 9. Copy Assets to Build Directory
file(COPY shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
