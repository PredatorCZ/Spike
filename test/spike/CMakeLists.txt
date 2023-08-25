add_subdirectory(compiled_resources)

build_target(
  NAME
  test_spike_cache
  TYPE
  APP
  SOURCES
  test_cache.cpp
  ${SPIKE_SOURCE_DIR}/src/cli/console.cpp
  LINKS
  spike-app-objects
  NO_PROJECT_H
  NO_VERINFO)

build_target(
  NAME
  test_spike_texel
  TYPE
  APP
  SOURCES
  test_texel.cpp
  ${SPIKE_SOURCE_DIR}/src/cli/console.cpp
  LINKS
  spike-app-objects
  NO_PROJECT_H
  NO_VERINFO)

install(TARGETS test_spike_texel test_spike_cache RUNTIME DESTINATION bin)
