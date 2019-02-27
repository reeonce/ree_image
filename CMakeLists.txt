cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project(ree_image CXX)

if(NOT TARGET ree_io)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ree_io
        ${CMAKE_CURRENT_BINARY_DIR}/ree_io)
endif(NOT TARGET ree_io)

set(REE_IMAGE_SRC
    src/ree/image/io/error.hpp
    src/ree/image/io/image.hpp
    src/ree/image/io/image.cpp
    src/ree/image/io/file_format.hpp
    src/ree/image/io/file_format.cpp
    src/ree/image/io/ppm.hpp
    src/ree/image/io/ppm.cpp
    src/ree/image/io/bmp.hpp
    src/ree/image/io/bmp.cpp
    src/ree/image/io/png.hpp
    src/ree/image/io/png.cpp
    src/ree/image/io/jpeg.hpp
    src/ree/image/io/jpeg.cpp

    src/ree/image/process/image.hpp
    src/ree/image/process/image.cpp
)

add_library(ree_image ${REE_IMAGE_SRC} ${REE_IO_SRC})
target_include_directories(ree_image PUBLIC src)
target_link_libraries(ree_image PUBLIC ree_io)
target_compile_features(ree_image PUBLIC cxx_std_11)

option(REE_IMAGE_WITH_ZLIB "whether to use zlib to decode png" ON)
if(REE_IMAGE_WITH_ZLIB)
    find_package(zlib REQUIRED) 
    target_link_libraries(ree_image PRIVATE ${ZLIB_LIBRARIES})
    target_include_directories(ree_image PRIVATE ${ZLIB_INCLUDE_DIRS})
endif(REE_IMAGE_WITH_ZLIB)

option(REE_IMAGE_ENABLE_SAMPLE "enable samples" OFF)
option(REE_IMAGE_ENABLE_TESTS "enable unit tests" OFF)

if(REE_IMAGE_ENABLE_SAMPLE)
    add_executable(ree_image_sample sample/sample.cc)
    target_link_libraries(ree_image_sample PRIVATE ree_image)
    
    add_custom_command(TARGET ree_image_sample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory test_assets ${CMAKE_CURRENT_BINARY_DIR}/test_assets)
endif(REE_IMAGE_ENABLE_SAMPLE)

if(REE_IMAGE_ENABLE_TESTS)
    if(NOT TARGET ree_unittest)
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ree_unittest
            ${CMAKE_CURRENT_BINARY_DIR}/ree_unittest)
    endif(NOT TARGET ree_unittest)

    set(REE_IMAGE_TESTS_SRC
        test/ree/image/test_config.h
        test/ree/image/png_tests.cc
        test/ree/image/jpeg_tests.cc
        test/ree/image/ppm_tests.cc
    )
    add_executable(ree_image_test test/test.cc ${REE_IMAGE_TESTS_SRC})
    target_include_directories(ree_image_test PRIVATE test/)
    target_link_libraries(ree_image_test PRIVATE ree_image ree_unittest)

    add_custom_command(TARGET ree_image_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${CMAKE_CURRENT_LIST_DIR}/test_assets ${CMAKE_CURRENT_BINARY_DIR}/test_assets)
endif(REE_IMAGE_ENABLE_TESTS)
