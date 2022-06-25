include(targetex)
include(odr_test)

set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL "" FORCE)
set(NO_OBJECTS
    ON
    CACHE BOOL "" FORCE)

test_odr(
  PATHS
  ${PROJECT_SOURCE_DIR}/datas
  EXCLUDE
  pugiex.hpp
  reflector_xml.hpp
  python
  encrypt)

target_link_libraries(test_odr precore)

build_target(
  NAME
  test_base
  TYPE
  APP
  SOURCES
  test_base.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

build_target(
  NAME
  test_reflector
  TYPE
  APP
  SOURCES
  test_reflector.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

build_target(
  NAME
  test_uni
  TYPE
  APP
  SOURCES
  test_uni.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

build_target(
  NAME
  test_app
  TYPE
  APP
  SOURCES
  test_app.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

if(NOT MINGW AND RELEASEVER)
  find_package(Python3 COMPONENTS Development Interpreter)

  if(Python3_FOUND)
    python3_add_library(test_unipy MODULE test_unipy.cpp
                        ${PRECORE_SOURCE_DIR}/uni/python/uni_python.cpp)

    target_link_libraries(test_unipy PRIVATE precore)
    install(TARGETS test_unipy LIBRARY DESTINATION bin)
    install(FILES test_uni.py DESTINATION bin)
  endif()
endif()

build_target(
  NAME
  test_xml
  TYPE
  APP
  SOURCES
  test_xml.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

build_target(
  NAME
  test_blowfish
  TYPE
  APP
  SOURCES
  test_blowfish.cpp
  LINKS
  precore
  NO_PROJECT_H
  NO_VERINFO)

add_subdirectory(shared)

install(TARGETS test_base test_app test_blowfish test_reflector test_uni
                test_xml RUNTIME DESTINATION bin)

add_subdirectory(../spike spike)
