add_definitions(-D_NOMINMAX)
add_definitions(-DGLEW_STATIC)
set(SOURCES
main.cpp
io_renderer_opengl.cpp
renderer_opengl.hpp
glew/glew.c
common/shader.cpp
  )

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} 
 "${CMAKE_CURRENT_SOURCE_DIR}/../freeglut-3.0.0/include" 
  "${CMAKE_SOURCE_DIR}/include" 
  "${CMAKE_CURRENT_SOURCE_DIR}/glew"
  "${CMAKE_CURRENT_SOURCE_DIR}/glm"
  "${CMAKE_CURRENT_SOURCE_DIR}/common"
  "../../include" 
)
add_executable(${FNAME}_opengl ${SOURCES})
target_link_libraries(${FNAME}_opengl ${PROJECT_NAME}_static ${OPENGL_LIBRARIES} freeglut)