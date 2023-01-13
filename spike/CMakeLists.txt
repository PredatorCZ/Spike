cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 23)
project(Spike VERSION 2.0)

build_target(
  NAME
  spike
  TYPE
  APP
  INCLUDES
  ../3rd_party/json
  LINKS
  precore
  DEFINITIONS
  MAIN_APP
  SOURCES
  spike.cpp
  out_context.cpp
  in_context.cpp
  context.cpp
  out_cache.cpp
  in_cache.cpp
  tmp_storage.cpp
  console.cpp
  batch.cpp
  AUTHOR
  "Lukas Cone"
  DESCR
  "Spike universal CLI interface"
  START_YEAR
  2021)

install(TARGETS spike RUNTIME DESTINATION bin)

if(NOT (WIN32 OR MINGW))
  target_link_libraries(spike dl)
endif()

if (MINGW)
  target_link_options(spike PRIVATE -municode)
endif()

function(add_spike_subdir name_)
  add_subdirectory(${name_} ${Spike_BINARY_DIR}/${name_})
endfunction()
