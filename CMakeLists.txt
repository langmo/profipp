cmake_minimum_required (VERSION 3.14)
project (PROFIPP VERSION 1.0.0)

set(PROFIPP_VERSION_MAJOR 1 CACHE STRING "major version" FORCE)
set(PROFIPP_VERSION_MINOR 0 CACHE STRING "minor version" FORCE)
set(PROFIPP_VERSION ${PROFIPP_VERSION_MAJOR}.${PROFIPP_VERSION_MINOR} CACHE STRING "version" FORCE)

# Source file extensions must be explicit (=do not append extensions to a source file if it couldn't be found)
cmake_policy(SET CMP0115 NEW)

# Default settings if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROFIPP)
  # Make option visible in ccmake, cmake-gui
  #option (BUILD_SHARED_LIBS "Build shared library" OFF)

  # Default to release build with debug info
  if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
  endif (NOT CMAKE_BUILD_TYPE)

  message(STATUS "Current build type is: ${CMAKE_BUILD_TYPE}")
  message(STATUS "Building for ${CMAKE_SYSTEM_NAME}")
endif()

# Setup install directories
set(include_dest "include/profipp${PROFIPP_VERSION}")
set(main_lib_dest "lib/profipp${PROFIPP_VERSION}")
set(lib_dest ${main_lib_dest}/${CMAKE_BUILD_TYPE})
set(bin_dest "bin")
message(STATUS "Install path: ${CMAKE_INSTALL_PREFIX}/${lib_dest}")
message(STATUS "Include path: ${CMAKE_INSTALL_PREFIX}/${include_dest}")
message(STATUS "Use the following cmake commands to include profipp in your project:")
message(STATUS "  find_package(profipp 1.0 REQUIRED)")
message(STATUS "  target_link_libraries(your_target PUBLIC profipp)")

# Always use standard .o suffix
set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

# Change default options for pnet
set(PNET_MAX_SLOTS            20
  CACHE STRING "Per API. Should be > 1 to allow at least one I/O module")
set(PNET_MAX_SUBSLOTS         10
  CACHE STRING "Per slot (DAP requires 2 + PNET_MAX_PHYSICAL_PORTS)")
  set(LOG_LEVEL ERROR CACHE STRING "default log level")

add_subdirectory (osal)
add_subdirectory (pnet)
add_subdirectory (profipp)
