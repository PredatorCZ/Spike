project(PreCoreTesting)

include(../cmake/targetex.cmake)
include(../cmake/chartype.cmake)

set(CMAKE_CXX_STANDARD 11)

build_target(
	TYPE APP
	SOURCES 
		datas.cpp
		${PRECORE_SOURCES}
	INCLUDES
		${PRECORE_SOURCE_DIR}
	NO_PROJECT_H
	NO_VERINFO
)

set_precore_sources(reflector reflector_io master_printer)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -msse3")
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