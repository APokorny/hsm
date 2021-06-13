cmake_policy(SET CMP0057 NEW)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)
cmake_minimum_required(VERSION 3.14)
project(hsm VERSION 0.2.0 LANGUAGES CXX)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++14 HAS_CXX14_FLAG)
check_cxx_compiler_flag(-std=c++17 HAS_CXX17_FLAG)
check_cxx_compiler_flag(-std=c++2a HAS_CXX20_FLAG)

include(FetchContent)
fetchcontent_declare(tiny_tuple
  GIT_REPOSITORY https://github.com/APokorny/tiny_tuple
  GIT_TAG development) # todo: pick a fixed revision

fetchcontent_makeavailable(tiny_tuple)

if(HAS_CXX20_FLAG)
  set(CMAKE_CXX_STANDARD 20)
elseif(HAS_CXX17_FLAG)
  set(CMAKE_CXX_STANDARD 17)
elseif(HAS_CXX14_FLAG)
  set(CMAKE_CXX_STANDARD 14)
else()
  message(FATAL_ERROR "requires at least c++14")
endif()

option(hsm_BUILD_TESTS "Build hsm examples and tests" ON)
add_library(hsm INTERFACE)
add_library(hsm::hsm ALIAS hsm)
target_link_libraries(hsm INTERFACE tiny_tuple)
target_include_directories(hsm INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    )
install(TARGETS hsm EXPORT hsm-Targets DESTINATION include)
install(EXPORT hsm-Targets
    NAMESPACE hsm::
    DESTINATION lib/cmake/hsm
    )
install(DIRECTORY include/hsm DESTINATION include)

if(hsm_BUILD_TESTS)
  add_subdirectory(example)
endif()
