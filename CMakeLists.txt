cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(ree_image CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(REE_IMAGE_SRC
    src/ree/image/error.h
    src/ree/image/image.h
    src/ree/image/image.cc
    src/ree/image/image_parser.h
    src/ree/image/image_parser.cc
    src/ree/image/ppm.h
    src/ree/image/ppm.cc
    src/ree/image/png_parser.h
    src/ree/image/png_parser.cc
)

add_library(ree_image ${REE_IMAGE_SRC})
target_include_directories(ree_image PUBLIC src/)

add_executable(sample sample.cc)
target_link_libraries(sample PRIVATE ree_image)

set(REE_IMAGE_TESTS_SRC
    test/ree/image/png_tests.h
    test/ree/image/png_tests.cc

    test/ree/image/jpeg_tests.h
    test/ree/image/jpeg_tests.cc

    test/ree/image/ppm_tests.h
    test/ree/image/ppm_tests.cc
)
add_executable(test test.cc ${REE_IMAGE_TESTS_SRC})
target_include_directories(test PRIVATE test/)
target_link_libraries(test PRIVATE ree_image)

add_custom_command(TARGET sample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory test_assets ${CMAKE_CURRENT_BINARY_DIR}/test_assets)
add_custom_command(TARGET test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
    copy_directory ${CMAKE_CURRENT_LIST_DIR}/test_assets ${CMAKE_CURRENT_BINARY_DIR}/test_assets)