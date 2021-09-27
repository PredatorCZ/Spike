include(targetex)
include(chartype)
include(odr_test)

set(PC_SHARED_LIB
    ON
    CACHE BOOL "" FORCE)

set(PUGI_SHARED_LIBS
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

if(${RELEASEVER} AND NOT MINGW)
  find_package(Python3 COMPONENTS Development Interpreter)

  if(${Python3_FOUND})
    build_target(
      NAME
      test_unipy
      TYPE
      PYMODULE
      SOURCES
      test_unipy.cpp
      LINKS
      precore
      NO_PROJECT_H
      NO_VERINFO)

    set_precore_sources(test_unipy uni_python)
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

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  message("Compiler is Clang")
elseif(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
  message("Compiler is GNU")
elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
  message("Compiler is MSVC")
else()
  message("Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
