add_executable(test_discrete_set test_discrete_set.cpp)
target_link_libraries(test_discrete_set ${PROJECT_NAME}_static)

add_executable(test_coordinates test_coordinates.cpp)
target_link_libraries(test_coordinates ${PROJECT_NAME}_static)

add_executable(test_aspheric test_aspheric.cpp)
target_link_libraries(test_aspheric ${PROJECT_NAME}_static)