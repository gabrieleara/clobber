cmake_minimum_required(VERSION 3.16)
project(clobber
    VERSION "0.1.0"
    HOMEPAGE_URL "https://github.com/gabrieleara/clobber"
    DESCRIPTION "Some helper functions for deterministic code execution (taken by google/benchmark, but without the benchmark)"
    LANGUAGES CXX
)
set(CPACK_PACKAGE_VENDOR "Gabriele Ara")

include(GNUInstallDirs)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(IS_TOPLEVEL_PROJECT TRUE)
else()
    set(IS_TOPLEVEL_PROJECT FALSE)
endif()

# ==================== GLOBAL OPTIONS ==================== #
# option(CLOBBER_OPT_INSTALL "Generate and install clobber target" ${IS_TOPLEVEL_PROJECT})
option(CLOBBER_OPT_BUILD_TESTS "Build tests" ${IS_TOPLEVEL_PROJECT})

# ================== PROJECT VARIABLES =================== #
set(INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(EXPORT_NAMESPACE "${PROJECT_NAME}::")

# if(NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
# if(NOT CMAKE_BUILD_TYPE)
# set(CMAKE_BUILD_TYPE "Release" CACHE STRING
# "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Coverage."
# FORCE)
# endif()
# endif()

# set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
# "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel.")

# ===================== MAIN TARGET ====================== #
add_library(${PROJECT_NAME} STATIC
    src/clobber.cpp)

add_library(${EXPORT_NAMESPACE}${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${INCLUDES}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if(CLOBBER_OPT_BUILD_TESTS)
    add_executable(test EXCLUDE_FROM_ALL
        test/test.cpp)

    target_link_libraries(test PUBLIC clobber)

    set_target_properties(clobber PROPERTIES
        LANGUAGE CXX
    )
endif()
