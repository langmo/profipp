add_library(profipp STATIC "")

target_include_directories(profipp
  PUBLIC
  $<BUILD_INTERFACE:${PROFIPP_SOURCE_DIR}/profipp/include> # for include headers when building
  $<INSTALL_INTERFACE:${include_dest}> # for include headers in install mode (for other projects to reuse profipp)
  PRIVATE
  src
  src/pugixml
  ${PNET_SOURCE_DIR}/include
  )

target_sources(profipp
  PRIVATE
  src/Profinet.cpp
  src/ProfinetInternal.cpp
  src/Device.cpp
  src/DeviceInstance.cpp
  src/Module.cpp
  src/ModuleInstance.cpp
  src/dapModule.cpp
  src/networktools.cpp
  src/Submodule.cpp
  src/SubmoduleInstance.cpp
  src/Parameter.cpp
  src/ParameterInstance.cpp
  src/Input.cpp
  src/InputInstance.cpp
  src/Output.cpp
  src/OutputInstance.cpp
  src/ConsoleLogger.cpp
  src/easylogging.cc
  src/FileLogger.cpp
  src/pugixml/pugixml.cpp
  src/gsdmltools.cpp
  )

target_compile_features(profipp PRIVATE cxx_std_17)
target_link_libraries (profipp PUBLIC pnet)

# Install compiled library to ${CMAKE_INSTALL_PREFIX}/${lib_dest} and 
# includes to ${CMAKE_INSTALL_PREFIX}/${include_dest}.
# CMAKE_INSTALL_PREFIX is usually /usr/local/ under Linux, so we e.g. install and include to
# /usr/local/lib/profipp-1.0/Release
# /usr/local/include/profipp-1.0/
install(TARGETS pnet EXPORT profipp DESTINATION  "${lib_dest}")
install(TARGETS osal EXPORT profipp DESTINATION  "${lib_dest}")
install(TARGETS profipp EXPORT profipp DESTINATION "${lib_dest}")
install(DIRECTORY include/ DESTINATION "${include_dest}")
# Create profipp-config.cmake and a profipp-config-version.cmake files in ${CMAKE_INSTALL_PREFIX}/${main_lib_dest} for other projects to easily reuse/include profipp
# profipp-config.cmake chooses the correct cmake file in one of the main_lib_dest subdirectories specified by lib_dest to include (created in the next step), 
# depending on the build type.
# profipp-config-version.cmake ensures that the requested major version of profipp matches the installed one,
# and that the requested minor version is smaller or equal to the installed one.
configure_file(profipp-config-version.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/profipp-config-version.cmake @ONLY)
install(FILES profipp-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/profipp-config-version.cmake DESTINATION ${main_lib_dest})
# Create profipp.cmake file in ${CMAKE_INSTALL_PREFIX}/${lib_dest} for other projects to easily reuse/include profipp
install(EXPORT profipp DESTINATION "${lib_dest}")

# Copy script to /usr/local/bin/ which is responsible for changing the IP address upon request by the profinet controller
install(FILES ${PROFIPP_SOURCE_DIR}/resources/profipp_network_parameters DESTINATION "${bin_dest}" PERMISSIONS WORLD_EXECUTE WORLD_READ)

# Search for compiler-provided system runtime libraries and add install rules for them
#include (InstallRequiredSystemLibraries)
include(InstallRequiredSystemLibraries)