cmake_minimum_required (VERSION 3.14)
project (PROFIPP VERSION 0.1.0)

# Default settings if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROFIPP)
  # Make option visible in ccmake, cmake-gui
  option (BUILD_SHARED_LIBS "Build shared library" OFF)

  # Default to release build with debug info
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  endif (NOT CMAKE_BUILD_TYPE)

  # Default to installing in build directory
  #if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  #  set(CMAKE_INSTALL_PREFIX ${PROFIPP_BINARY_DIR}/install
  #    CACHE PATH "Default install path" FORCE)
  #endif()

  message(STATUS "Current build type is: ${CMAKE_BUILD_TYPE}")
  message(STATUS "Current install path is: ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "Building for ${CMAKE_SYSTEM_NAME}")
endif()

# Always use standard .o suffix
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

add_subdirectory (osal)
add_subdirectory (pnet)
add_subdirectory (profipp)
add_subdirectory (examples/profiecho)
add_subdirectory (examples/profizumo)
