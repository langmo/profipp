#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2017 rt-labs AB, Sweden.
#
# This software is licensed under the terms of the BSD 3-clause
# license. See the file LICENSE distributed with this software for
# full license information.
#*******************************************************************/

cmake_minimum_required (VERSION 3.14)
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/tools")
project (OSAL VERSION 0.1.0)

#include(GetGitRevision)

# Default settings if this is the main project
#if (CMAKE_PROJECT_NAME STREQUAL OSAL)
#  include(CTest)
#
#  # Default to release build with debug info
#  if (NOT CMAKE_BUILD_TYPE)
#    set (CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING
#      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
#      FORCE)
#  endif (NOT CMAKE_BUILD_TYPE)
#
#  # Default to installing in build directory
#  if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#    set(CMAKE_INSTALL_PREFIX ${OSAL_BINARY_DIR}/install
#      CACHE PATH "Default install path" FORCE)
#  endif()
#
#  message(STATUS "Current build type is: ${CMAKE_BUILD_TYPE}")
#  message(STATUS "Current install path is: ${CMAKE_INSTALL_PREFIX}")
#  message(STATUS "Building for ${CMAKE_SYSTEM_NAME}")
#endif()

# Always use standard .o suffix
#set(CMAKE_C_OUTPUT_EXTENSION_REPLACE 1)
#set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

# Generate version numbers
configure_file (
  version.h.in
  ${OSAL_BINARY_DIR}/src/version.h
  )

# Add platform-dependent targets early, so they can be configured by
# platform
add_library(osal "")

# Use position independent code if platform supports shared libraries
get_property(SUPPORTS_SHARED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
if (SUPPORTS_SHARED)
  set_property(TARGET osal PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()


# Platform configuration
include(${OSAL_SOURCE_DIR}/cmake/${CMAKE_SYSTEM_NAME}.cmake)

set_target_properties (osal
  PROPERTIES
  C_STANDARD 99
  )

target_compile_features(osal PUBLIC c_std_99)

target_include_directories(osal
  PUBLIC
  $<BUILD_INTERFACE:${OSAL_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  src
  ${OSAL_BINARY_DIR}/src
  )

install(
  TARGETS osal
  EXPORT OsalTargets
  )

install(
  EXPORT OsalTargets
  FILE OsalTargets.cmake
  DESTINATION cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/OsalConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/OsalConfig.cmake"
  INSTALL_DESTINATION cmake
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/OsalConfig.cmake
  DESTINATION cmake
  )

install(FILES
  include/osal.h
  include/osal_log.h
  DESTINATION include
  )