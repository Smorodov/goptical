set(SOURCES  opengl_app.cpp)

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

find_package(OpenGL REQUIRED)
#find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}   "${CMAKE_CURRENT_SOURCE_DIR}/../freeglut-3.0.0/include" )

find_package( OpenCV REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS} "../freeglut-3.0.0")

add_executable(${FNAME}_opengl ${SOURCES})
target_link_libraries(${FNAME}_opengl ${PROJECT_NAME}_static ${OPENGL_LIBRARIES} freeglut ${OpenCV_LIBRARIES})
