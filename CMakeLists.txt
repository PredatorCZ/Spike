cmake_minimum_required (VERSION 3.3)

project(PreCoreTesting)


set (UNICODE false)
set (RELEASEVER false)

if (RELEASEVER)
	set(CMAKE_BUILD_TYPE Release)
else()
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 11)

add_executable (PreCoreTesting "_samples/datas.cpp" "datas/reflector.cpp" "datas/MasterPrinter.cpp")
include_directories("./")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if (UNICODE)
	add_definitions(-D_UNICODE -DUNICODE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT} -msse3")
if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
	message("Compiler is Clang")
elseif(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof ${CMAKE_THREAD_LIBS_INIT}")
	message("Compiler is GNU")
else()
	message("Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()
