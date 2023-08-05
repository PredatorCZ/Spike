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
  SOURCES
  spike.cpp
  console.cpp
  AUTHOR
  "Lukas Cone"
  DESCR
  "Spike universal CLI interface"
  START_YEAR
  2021)


install(TARGETS spike_cli RUNTIME DESTINATION $<IF:$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${SPIKE_NODOT}>>>,.,bin>)

function(add_spike_subdir name_)
  add_subdirectory(${name_} ${SpikeCLI_BINARY_DIR}/${name_})
endfunction()
