cmake_minimum_required (VERSION 3.14)
project (PROFIECHO VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
# Otherwise, the build executable will find profiecho.so, but not the installed one.
# This seems to be some strange behavior. Also, this has to be done before any target has been defined, 
# i.e. before add_executable(..)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(profiecho src/profiecho.cpp)

target_include_directories(profiecho
  PRIVATE
  src
  )

target_sources(profiecho
  PRIVATE
  src/EchoControl.cpp
)

configure_file(${PROFIECHO_SOURCE_DIR}/resources/set_network_parameters ${PROFIECHO_BINARY_DIR}/tmp/set_network_parameters COPYONLY)
file (COPY ${PROFIECHO_BINARY_DIR}/tmp/set_network_parameters DESTINATION ${PROFIECHO_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
file(REMOVE_RECURSE ${PROFIECHO_BINARY_DIR}/tmp/)

target_compile_features(profiecho PRIVATE cxx_std_17)

find_package(profipp 1.0 REQUIRED)
target_link_libraries(profiecho PUBLIC profipp)

install(TARGETS profiecho DESTINATION bin)
install(FILES ${PROFIECHO_BINARY_DIR}/set_network_parameters DESTINATION bin PERMISSIONS WORLD_EXECUTE)

include(InstallRequiredSystemLibraries)
