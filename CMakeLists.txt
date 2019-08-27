cmake_minimum_required (VERSION 3.3)


set (UNICODE false)
set (RELEASEVER false)

if (RELEASEVER)
	set(CMAKE_BUILD_TYPE Release)
else()
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 11)

project(PreCoreTesting)
add_executable (PreCoreTesting "_samples/datas.cpp" "datas/reflector.cpp" "datas/MasterPrinter.cpp")
include_directories("./")

target_link_libraries(${PROJECT_NAME} Threads::Threads)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

if (UNICODE)
	add_definitions(-D_UNICODE -DUNICODE)
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
endif()
