file(GLOB_RECURSE SOURCES "./*.cpp")
include_directories("./include")

foreach(TEST_FILE ${SOURCES})
    GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
    SET(TEST_NAME "xtest_${TEST_NAME}")
    
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} ${DEPENDENCIES})
    add_test(${TEST_NAME} ${TEST_NAME})
endforeach()
