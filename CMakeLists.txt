cmake_minimum_required(VERSION 3.20)

project(playground CXX)

# ========================================================================== #
set(san "" CACHE STRING "Add sanitizer instrumentation")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set_property(CACHE san PROPERTY STRINGS ";undefined;memory;address;thread")
# ========================================================================== #

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_library(clio_options INTERFACE)
target_compile_features(clio_options INTERFACE cxx_std_23) # Clio needs c++23 but deps can remain c++20 for now
target_include_directories(clio_options INTERFACE ${CMAKE_SOURCE_DIR}/src)

include(Settings)

include(deps/Boost)
include(deps/gtest)

# Enable selected sanitizer if enabled via `san`
if (san)
  set(SUPPORTED_SANITIZERS "address" "thread" "memory" "undefined")
  if (NOT san IN_LIST SUPPORTED_SANITIZERS)
    message(FATAL_ERROR "Error: Unsupported sanitizer '${san}'. Supported values are: ${SUPPORTED_SANITIZERS}.")
  endif ()

  # Sanitizers recommend minimum of -O1 for reasonable performance so we enable it for debug builds
  set(SAN_OPTIMIZATION_FLAG "")
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SAN_OPTIMIZATION_FLAG -O1)
  endif ()
  target_compile_options(clio_options INTERFACE ${SAN_OPTIMIZATION_FLAG} ${SAN_FLAG} -fno-omit-frame-pointer)

  target_compile_definitions(
    clio_options INTERFACE $<$<STREQUAL:${san},address>:SANITIZER=ASAN> $<$<STREQUAL:${san},thread>:SANITIZER=TSAN>
                           $<$<STREQUAL:${san},memory>:SANITIZER=MSAN> $<$<STREQUAL:${san},undefined>:SANITIZER=UBSAN>
  )
  target_link_libraries(clio_options INTERFACE ${SAN_FLAG} ${SAN_LIB})
endif ()

add_executable(playground_tests tests/unit/Main.cpp tests/unit/timer.cpp)
target_link_libraries(playground_tests clio_options boost::boost gtest::gtest)
