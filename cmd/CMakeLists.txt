set(SOURCES
        generic_lens.cpp
  )

add_executable(gopt ${SOURCES})
target_link_libraries(gopt ${PROJECT_NAME}_static)
