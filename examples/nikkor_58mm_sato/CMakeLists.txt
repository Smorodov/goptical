set(SOURCES
  nikkor_58mm_sato.cpp
  )

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${FNAME} ${SOURCES})
target_link_libraries(${FNAME} ${PROJECT_NAME}_static)
