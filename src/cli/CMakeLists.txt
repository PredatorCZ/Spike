set(CMAKE_CXX_STANDARD 23)
project(SpikeCLI VERSION 2.0)
set(CMAKE_INSTALL_RPATH "$ORIGIN/lib")

build_target(
  NAME
  spike_cli
  TYPE
  APP
  LINKS
  spike-app-objects
  INCLUDES
  ${SPIKE_SOURCE_DIR}/3rd_party/json
  SOURCES
  spike.cpp
  console.cpp
  AUTHOR
  "Lukas Cone"
  DESCR
  "Spike universal CLI interface"
  START_YEAR
  2021)

target_compile_options(spike_cli PRIVATE -fvisibility=hidden)

if(NOT BUILD_SHARED_LIBS)
  target_link_libraries(spike_cli spike-objects pugixml-objects -rdynamic)
  set_target_properties(spike_cli PROPERTIES ENABLE_EXPORTS ON)

  if(GLTF)
    target_link_libraries(spike_cli gltf-objects)
  endif()
endif()

install(TARGETS spike_cli RUNTIME DESTINATION $<IF:$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${SPIKE_NODOT}>>>,.,bin>)

function(add_spike_subdir name_)
  add_subdirectory(${name_} ${SpikeCLI_BINARY_DIR}/${name_})
endfunction()
