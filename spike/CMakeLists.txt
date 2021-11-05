cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
project(Spike VERSION 1.0)

build_target(
  NAME
  spike
  TYPE
  APP
  LINKS
  precore
  DEFINITIONS
  MAIN_APP
  SOURCES
  spike.cpp
  out_context
  in_context
  context
  out_cache
  in_cache
  tmp_storage
  AUTHOR
  "Lukas Cone"
  DESCR
  "Spike universal CLI interface"
  START_YEAR
  2021)

install(TARGETS spike RUNTIME DESTINATION bin)

if(NOT MSVC)
  target_link_libraries(spike dl)
endif()

function(add_spike_subdir name_)
  add_subdirectory(${name_} ${Spike_BINARY_DIR}/${name_})
endfunction()
