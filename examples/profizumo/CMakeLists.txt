cmake_minimum_required (VERSION 3.14)
project (PROFIZUMO VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)
add_executable(profizumo src/profizumo.cpp)

target_include_directories(profizumo
  PRIVATE
  src
  ${PROFIPP_SOURCE_DIR}/include
  )

target_sources(profizumo
  PRIVATE
  src/ZumoControl.cpp
  src/SerialConnection.cpp
)

configure_file(${PROFIZUMO_SOURCE_DIR}/resources/set_network_parameters ${PROFIZUMO_BINARY_DIR}/tmp/set_network_parameters COPYONLY)
file (COPY ${PROFIZUMO_BINARY_DIR}/tmp/set_network_parameters DESTINATION ${PROFIZUMO_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE ${PROFIZUMO_BINARY_DIR}/tmp/)

target_compile_features(profizumo PRIVATE cxx_std_17)
target_link_libraries (profizumo PUBLIC pnet)
target_link_libraries (profizumo PUBLIC profipp)

install (TARGETS profizumo DESTINATION bin)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${PROFIZUMO_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_CONTACT moritz.lang@technikum-wien.at)
include (CPack)
