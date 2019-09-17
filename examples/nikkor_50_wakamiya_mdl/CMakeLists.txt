set(SOURCES
  nikkor_wakamiya_50_1_4_mdl.cpp
  )

get_filename_component( FNAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${FNAME} ${SOURCES})
target_link_libraries(${FNAME} ${PROJECT_NAME}_static)
