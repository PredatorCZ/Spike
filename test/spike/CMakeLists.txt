build_target(
  NAME
  test_spike_cache
  TYPE
  APP
  SOURCES
  test_cache.cpp
  ${SPIKE_SOURCE_DIR}/src/app/out_cache.cpp
  ${SPIKE_SOURCE_DIR}/src/app/in_cache.cpp
  ${SPIKE_SOURCE_DIR}/src/cli/console.cpp
  LINKS
  spike
  NO_PROJECT_H
  NO_VERINFO)

install(TARGETS test_spike_cache RUNTIME DESTINATION bin)
