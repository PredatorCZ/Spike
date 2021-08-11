cmake_minimum_required(VERSION 3.12)
project(PreCore-mgw64-test)
enable_testing()

add_test(test_base test_base)
add_test(test_reflector test_reflector)
add_test(test_uni test_uni)
add_test(
  NAME test_app
  COMMAND bin/test_app
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/..)

find_package(Python3 COMPONENTS Interpreter)

add_test(NAME test_unipy COMMAND ${Python3_EXECUTABLE} test_uni.py)

add_test(test_xml test_xml)
add_test(test_blowfish test_blowfish)
add_test(shared_test shared_test)
