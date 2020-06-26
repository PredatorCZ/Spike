project(test_base)

include(../cmake/targetex.cmake)
include(../cmake/chartype.cmake)
set(CMAKE_CXX_STANDARD 11)

build_target(
	TYPE APP
	SOURCES 
		test_base.cpp
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources(master_printer matrix44)

project(test_reflector)

build_target(
	TYPE APP
	SOURCES 
		test_reflector.cpp
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources(reflector reflector_io master_printer)

project(test_uni)

build_target(
	TYPE APP
	SOURCES 
		test_uni.cpp
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources(master_printer uni_format)

project(test_app)

build_target(
	TYPE APP
	SOURCES 
		test_app.cpp
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources(master_printer directory_scanner reflector)
enable_threads()

enable_testing()
add_test(test_base test_base)
add_test(test_reflector test_reflector)
add_test(test_uni test_uni)
add_test(NAME test_app COMMAND test_app WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})


if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	message("Compiler is Clang")
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	message("Compiler is GNU")
elseif (CMAKE_CXX_COMPILER_ID MATCHES MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")
	message("Compiler is MSVC")
else()
	message("Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()