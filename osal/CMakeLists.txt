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
project (OSAL VERSION 0.1.0)

# Generate version numbers
configure_file (
  version.h.in
  ${OSAL_BINARY_DIR}/src/version.h
  )

# Add platform-dependent targets early, so they can be configured by
# platform
add_library(osal STATIC "")

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