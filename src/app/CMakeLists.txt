set(CMAKE_CXX_STANDARD 23)

include(targetex)

build_target(
  NAME
  spike-app-objects
  TYPE
  OBJECT
  INCLUDES
  ${SPIKE_SOURCE_DIR}/3rd_party/json
  LINKS
  spike
  DEFINITIONS
  MAIN_APP
  SOURCES
  batch.cpp
  context.cpp
  in_cache.cpp
  in_context.cpp
  out_cache.cpp
  out_context.cpp
  pvr_decompress.cpp
  bc7decomp.c
  texel.cpp
  tmp_storage.cpp
  NO_PROJECT_H
  NO_VERINFO
)

if(NOT (WIN32 OR MINGW))
  target_link_libraries(spike-app-objects dl)
endif()

if (MINGW)
  target_link_options(spike-app-objects PRIVATE -municode)
endif()
