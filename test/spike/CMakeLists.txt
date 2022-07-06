build_target(
  NAME
  test_spike_cache
  TYPE
  APP
  SOURCES
  test_cache.cpp
  ${PRECORE_SOURCE_DIR}/spike/out_cache.cpp
  ${PRECORE_SOURCE_DIR}/spike/in_cache.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

install(TARGETS test_spike_cache RUNTIME DESTINATION bin)
