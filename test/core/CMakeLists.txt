set(SOURCES
  test_discrete_set.cpp
  )

add_executable(test_discrete_set ${SOURCES})
target_link_libraries(test_discrete_set ${PROJECT_NAME}_static)