cmake_minimum_required (VERSION 3.14)
project (PROFIECHO VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)
add_executable(profiecho src/profiecho.cpp)

target_include_directories(profiecho
  PRIVATE
  src
  ${PROFINETPP_SOURCE_DIR}/include
  )

target_sources(profiecho
  PRIVATE
  src/EchoControl.cpp
)

configure_file(${PROFIECHO_SOURCE_DIR}/resources/set_network_parameters ${PROFIECHO_BINARY_DIR}/tmp/set_network_parameters COPYONLY)
file (COPY ${PROFIECHO_BINARY_DIR}/tmp/set_network_parameters DESTINATION ${PROFIECHO_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE ${PROFIECHO_BINARY_DIR}/tmp/)

target_compile_features(profiecho PRIVATE cxx_std_17)
target_link_libraries (profiecho PUBLIC profinet)
target_link_libraries (profiecho PUBLIC profinetpp)

install (TARGETS profiecho DESTINATION bin)

include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${PROFIECHO_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_CONTACT moritz.lang@technikum-wien.at)
include (CPack)
