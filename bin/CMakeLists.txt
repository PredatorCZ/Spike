cmake_minimum_required(VERSION 3.12)
project(PreCore-test)
enable_testing()

add_test(test_base test_base)
add_test(test_reflector test_reflector)
add_test(test_uni test_uni)
add_test(
  NAME test_app
  COMMAND bin/test_app
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

add_test(test_xml test_xml)
add_test(test_blowfish test_blowfish)
add_test(shared_test shared_test)

add_test(
  NAME test_spike_cache
  COMMAND bin/test_spike_cache
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

add_dependencies(test_xml test_reflector)

if(NOT MINGW)
  find_package(Python3 COMPONENTS Interpreter)
  add_test(NAME test_unipy COMMAND ${Python3_EXECUTABLE} test_uni.py)
endif()
