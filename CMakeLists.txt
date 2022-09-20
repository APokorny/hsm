cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_minimum_required(VERSION 3.14)
project(hsm VERSION 0.2.0 LANGUAGES CXX)

include(cmake/get_cpm.cmake)
CPMAddPackage(
  NAME tiny_tuple
  GITHUB_REPOSITORY APokorny/tiny_tuple
  GIT_TAG 4812fdf1f05b32616720616d943ba8867dde982e
  )
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.9.0")
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(hsm_STANDALONE ON)
else()
  set(hsm_STANDALONE OFF)
endif()

option(hsm_BUILD_TESTS "Build hsm examples and tests" ${HSM_STANDALONE})
add_library(hsm INTERFACE)
add_library(hsm::hsm ALIAS hsm)
target_compile_features(hsm INTERFACE cxx_std_17)
target_link_libraries(hsm INTERFACE tiny_tuple)
target_include_directories(hsm INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  )
packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  DEPENDENCIES "tiny_tuple 0.1.0"
  VERSION_HEADER "${PROJECT_NAME}/version.h"
  NAMESPACE hsm
  COMPATIBILITY AnyNewerVersion
  DISABLE_VERSION_SUFFIX YES
  ARCH_INDEPENDENT YES
  )

if(hsm_BUILD_TESTS)
  CPMAddPackage("gh:catchorg/Catch2@2.13.7")
  add_subdirectory(example)
endif()
