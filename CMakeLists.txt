cmake_minimum_required(VERSION 3.0)
project(OpenGL-sandbox)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(lib/glfw)

option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(lib/assimp)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /std:c++17")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++17")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(src/engine/
					src/model/
					src/scene/
					src/gui/
					src/camera
					src/processing/
                    lib/assimp/include/
                    lib/glad/include/
                    lib/glfw/include/
                    lib/glm/
                    lib/stb/
					lib/imgui/
					lib/json/)

file(GLOB LIB_SOURCES lib/glad/src/glad.c
					  lib/imgui/*.cpp)

# Header files
file(GLOB PROJECT_HEADERS src/engine/*.hpp 
						  src/model/*.hpp
						  src/scene/*.hpp
						  src/gui/*.hpp
						  src/processing/*.hpp
						  src/camera/.*hpp)

# Source files
file(GLOB PROJECT_SOURCES src/engine/*.cpp 
                          src/model/*.cpp
						  src/scene/*.cpp
						  src/gui/*.cpp
						  src/processing/*.cpp
						  src/camera/*.cpp)

file(GLOB PROJECT_SHADERS resources/shaders/*.glsl
                          resources/shaders/*.frag
                          resources/shaders/*.vert
                          resources/shaders/lighting/*.glsl
                          resources/shaders/lighting/*.frag
                          resources/shaders/lighting/*.vert
                          resources/shaders/lighting/ibl/*.glsl
                          resources/shaders/lighting/ibl/*.frag
                          resources/shaders/lighting/ibl/*.vert
                          resources/shaders/postprocess/*.glsl
                          resources/shaders/postprocess/*.frag
                          resources/shaders/postprocess/*.vert)

file(GLOB PROJECT_CONFIGS CMakeLists.txt
                          README.md)
#                         .gitattributes
#                         .gitignore
#                         .gitmodules)

source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("LIB" FILES ${LIB_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE
                -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADERS}
                               ${PROJECT_SHADERS} ${PROJECT_CONFIGS} 
							   ${LIB_SOURCES})
target_link_libraries(${PROJECT_NAME} assimp glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
