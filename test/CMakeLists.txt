include(../cmake/targetex.cmake)
include(../cmake/chartype.cmake)
include(../cmake/odr_test.cmake)
set(PC_SHARED_LIB
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

if(MSVC)
  add_custom_command(TARGET test_odr COMMAND setx PATH "$<TARGET_FILE_DIR:precore>";)
endif()

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

add_test(test_base test_base)

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

add_test(test_reflector test_reflector)

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

add_test(test_uni test_uni)

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
      LINKS
      precore
      NO_PROJECT_H
      NO_VERINFO)

    set_precore_sources(test_unipy uni_python)

    add_test(
      NAME test_unipy
      COMMAND ${Python2_EXECUTABLE} test_uni.py
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test)

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
  pugixml-shared
  precore
  NO_PROJECT_H
  NO_VERINFO)

add_test(test_xml test_xml)

add_subdirectory(shared)

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
