# profipp-config.cmake - package configuration file
# This file allows other projects use find_package(profipp ...) and should be installed to
# ${CMAKE_INSTALL_PREFIX}/lib/profipp-[major].[minor]/ .
# find_package() will then add this cmake file, which will again add the correct
# cmake file dependent on the build type, e.g.
# ${CMAKE_INSTALL_PREFIX}/lib/profipp-[major].[minor]/Release/profipp.cmake .
get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/${CMAKE_BUILD_TYPE}/profipp.cmake)