set(SOURCES
  opengl_app.cpp
  )

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_subdirectory("freeglut-3.0.0")
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} 
 "${CMAKE_CURRENT_SOURCE_DIR}/freeglut-3.0.0/include" 
  "${CMAKE_SOURCE_DIR}/include" 
  "../../include" 
)
add_executable(${FNAME}_opengl ${SOURCES})
target_link_libraries(${FNAME}_opengl ${PROJECT_NAME}_static ${OPENGL_LIBRARIES} freeglut)