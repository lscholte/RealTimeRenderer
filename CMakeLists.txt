cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(openGL_realtime_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR} ${GLM_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARIES} ${SDL2_LIBRARIES})

file(GLOB RENDERER_SRC "*.h" "*.cpp" "*.c")

add_executable(renderer ${RENDERER_SRC})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})