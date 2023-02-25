cmake_minimum_required (VERSION 3.14)
project (PROFIPP VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)
add_library(profipp "")

target_include_directories(profipp
  PRIVATE
  src
  src/pugixml
  include
  ${PROFINET_SOURCE_DIR}/include
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

install (TARGETS profipp DESTINATION lib)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${PROFIPP_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_CONTACT moritz.lang@technikum-wien.at)
include (CPack)
