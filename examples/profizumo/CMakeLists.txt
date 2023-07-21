cmake_minimum_required (VERSION 3.14)
project (PROFIZUMO VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
# Otherwise, the build executable will find profiecho.so, but not the installed one.
# This seems to be some strange behavior. Also, this has to be done before any target has been defined, 
# i.e. before add_executable(..)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(profizumo src/profizumo.cpp)

target_include_directories(profizumo
  PRIVATE
  src
  resources/profizumo
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

find_package(profipp 1.0 REQUIRED)
target_link_libraries (profizumo PUBLIC profipp)

install (TARGETS profizumo DESTINATION bin)
install(FILES ${PROFIZUMO_BINARY_DIR}/set_network_parameters DESTINATION bin PERMISSIONS WORLD_EXECUTE)

include (InstallRequiredSystemLibraries)
