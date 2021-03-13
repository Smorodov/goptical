add_executable(test_discrete_set test_discrete_set.cpp)
target_link_libraries(test_discrete_set ${PROJECT_NAME}_static)

add_executable(test_coordinates test_coordinates.cpp)
target_link_libraries(test_coordinates ${PROJECT_NAME}_static)

add_executable(test_aspheric test_aspheric.cpp)
target_link_libraries(test_aspheric ${PROJECT_NAME}_static)

add_executable(test_shapes test_shapes.cpp)
target_link_libraries(test_shapes ${PROJECT_NAME}_static)

add_executable(test_patterns test_patterns.cpp)
target_link_libraries(test_patterns ${PROJECT_NAME}_static)

add_executable(test_materials test_materials.cpp)
target_link_libraries(test_materials ${PROJECT_NAME}_static)

add_executable(test_rendering test_rendering.cpp)
target_link_libraries(test_rendering ${PROJECT_NAME}_static)
