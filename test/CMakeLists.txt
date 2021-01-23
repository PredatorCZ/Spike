include(../cmake/targetex.cmake)
include(../cmake/chartype.cmake)
include(../cmake/odr_test.cmake)
set(CMAKE_CXX_STANDARD 14)

test_odr(
  PATHS
  ${PROJECT_SOURCE_DIR}/datas
  EXCLUDE
  pugiex.hpp
  reflector_xml.hpp
  python
  encrypt)

set_precore_sources(test_odr master_printer)

build_target(
  NAME
  test_base
  TYPE
  APP
  SOURCES
  test_base.cpp
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_base master_printer matrix44 multi_thread)
add_test(test_base test_base)

build_target(
  NAME
  test_reflector
  TYPE
  APP
  SOURCES
  test_reflector.cpp
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_reflector reflector reflector_io master_printer)
add_test(test_reflector test_reflector)

build_target(
  NAME
  test_uni
  TYPE
  APP
  SOURCES
  test_uni.cpp
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_uni master_printer uni_format)
add_test(test_uni test_uni)

build_target(
  NAME
  test_app
  TYPE
  APP
  SOURCES
  test_app.cpp
  NO_PROJECT_H
  NO_VERINFO)

set_precore_sources(test_app master_printer directory_scanner reflector)
add_test(
  NAME test_app
  COMMAND test_app
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

if(${RELEASEVER})
  find_package(Python2 COMPONENTS Development Interpreter)

  if(${Python2_FOUND})
    build_target(
      NAME
      test_unipy
      TYPE
      PYMODULE
      SOURCES
      test_unipy.cpp
      NO_PROJECT_H
      NO_VERINFO)

    set_precore_sources(test_unipy uni_format uni_python matrix44)
    add_test(
      NAME test_unipy
      COMMAND ${Python2_EXECUTABLE} test_uni.py
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)
  endif()
endif()

find_package(pugixml)

if(${pugixml_FOUND})
  build_target(
    NAME
    test_xml
    TYPE
    APP
    SOURCES
    test_xml.cpp
    LINKS
    pugixml
    NO_PROJECT_H
    NO_VERINFO)

  set_precore_sources(test_xml master_printer reflector reflector_xml reflector_io)
  add_test(test_xml test_xml)
endif()

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
