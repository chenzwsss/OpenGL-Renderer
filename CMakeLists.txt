cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME OpenGL_Renderer)

project (${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17) # this does nothing for MSVC, use target_compile_options below
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

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
    ${PROJECT_SOURCE_DIR}/3rdParty/glad/src/glad.c
    src/graphic/gl_vertex_array.cpp
    src/utility/resource_manager.cpp
    src/skybox.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdParty/glfw)
set(LIBS ${LIBS} glfw)

# glm
add_subdirectory(3rdParty/glm)
set(LIBS ${LIBS} glm)

# imgui
add_subdirectory(3rdParty/imgui)
set(LIBS ${LIBS} imgui)

# stb_image
add_library(stb_image ${PROJECT_SOURCE_DIR}/3rdParty/stb/stb_image.cpp)
set(LIBS ${LIBS} stb_image)

# assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS  OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX  OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW  OFF CACHE BOOL "" FORCE)
add_subdirectory(3rdParty/assimp)
set(LIBS ${LIBS} assimp)

target_link_libraries(${PROJECT_NAME} ${LIBS})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/src
        ${PROJECT_SOURCE_DIR}/3rdParty/glad/include
        ${PROJECT_SOURCE_DIR}/3rdParty/glfw/include
        ${PROJECT_SOURCE_DIR}/3rdParty/glm
        ${PROJECT_SOURCE_DIR}/3rdParty/stb
        ${PROJECT_SOURCE_DIR}/3rdParty/imgui
        ${PROJECT_SOURCE_DIR}/3rdParty/assimp/include
)