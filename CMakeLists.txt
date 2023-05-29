cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME OpenGL-Renderer)

project (${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build(Debug or Release)" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if(WIN32)
    set(LIBS opengl32)
elseif(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
    find_package(OpenGL REQUIRED)
    add_definitions(${OPENGL_DEFINITIONS})
    find_package(X11 REQUIRED)
    # note that the order is important for setting the libs
    # use pkg-config --libs $(pkg-config --print-requires --print-requires-private glfw3) in a terminal to confirm
    set(LIBS X11 Xrandr Xinerama Xi Xxf86vm Xcursor GL dl pthread)
    set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
elseif(APPLE)
    INCLUDE_DIRECTORIES(/System/Library/Frameworks)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
    FIND_LIBRARY(IOKit_LIBRARY IOKit)
    FIND_LIBRARY(CoreVideo_LIBRARY CoreVideo)
    MARK_AS_ADVANCED(COCOA_LIBRARY OpenGL_LIBRARY)
    SET(APPLE_LIBS ${COCOA_LIBRARY} ${IOKit_LIBRARY} ${OpenGL_LIBRARY} ${CoreVideo_LIBRARY})
    SET(APPLE_LIBS ${APPLE_LIBS})
    set(LIBS ${LIBS} ${APPLE_LIBS})
else()
    set(LIBS )
endif(WIN32)

set(SOURCES
    src/main.cpp
    ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c
    src/graphic/GLVertexArray.h
    src/graphic/GLVertexArray.cpp
    src/graphic/GLShaderProgram.h
    src/graphic/GLShaderProgram.cpp
    src/graphic/ShaderCreateInfo.h
    src/utility/ResourceManager.h
    src/utility/ResourceManager.cpp
    src/utility/ImGUIRenderer.h
    src/utility/ImGUIRenderer.cpp
    src/base/RenderCamera.hpp
    src/base/Vertex.h
    src/base/Skybox.h
    src/base/Skybox.cpp
    src/base/glTFModel.h
    src/base/glTFModel.cpp
    src/base/glTFMesh.h
    src/base/glTFMesh.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)
set(LIBS ${LIBS} glfw)

# glm
add_subdirectory(external/glm)
set(LIBS ${LIBS} glm)

# imgui
add_subdirectory(external/imgui)
set(LIBS ${LIBS} imgui)

# tinygltf
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_GL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_BUILDER_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(external/tinygltf)
set(LIBS ${LIBS} tinygltf)

target_link_libraries(${PROJECT_NAME} ${LIBS})

include_directories(src)
include_directories(external/glad/include)
include_directories(external/glfw/include)
include_directories(external/glm)
include_directories(external/stb)
include_directories(external/imgui)
include_directories(external/assimp/include)
include_directories(external/tinygltf)

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
