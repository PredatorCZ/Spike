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
  out_context.cpp
  in_context.cpp
  context.cpp
  out_cache.cpp
  in_cache.cpp
  tmp_storage.cpp
  batch.cpp
  NO_PROJECT_H
  NO_VERINFO
)

if(NOT (WIN32 OR MINGW))
  target_link_libraries(spike-app-objects dl)
endif()

if (MINGW)
  target_link_options(spike-app-objects PRIVATE -municode)
endif()
