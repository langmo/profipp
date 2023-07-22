cmake_minimum_required (VERSION 3.14)
project (PROFIECHO VERSION 0.1.0)

cmake_policy(SET CMP0115 NEW)

add_executable(profiecho src/profiecho.cpp)
target_include_directories(profiecho
  PRIVATE
  src
)
target_sources(profiecho
  PRIVATE
  src/EchoControl.cpp
)

# Copy script which is responsible to set device's IP address upon profinet controller request to build tree.
# Note that this is only necessary if one wants to run the build files. The install files search at the correct position.
file (COPY ${CMAKE_INSTALL_PREFIX}/bin/profipp_network_parameters DESTINATION ${PROFIECHO_BINARY_DIR}/ FILE_PERMISSIONS WORLD_EXECUTE)

target_compile_features(profiecho PRIVATE cxx_std_17)

# Add thread support (needed by profi++)
find_package(Threads REQUIRED)
target_link_libraries(profiecho INTERFACE Threads::Threads)

# Add profinet support via profipp v1.0 or higher
find_package(profipp 1.0 REQUIRED)
#target_link_libraries(profiecho PUBLIC /usr/local/lib/profipp1.0/Release/libpnet.a)
target_link_libraries(profiecho PUBLIC profipp)

# Install the example application
install(TARGETS profiecho DESTINATION bin)
# Also install any additional files which might be required due to anything compiler specific
include(InstallRequiredSystemLibraries)
