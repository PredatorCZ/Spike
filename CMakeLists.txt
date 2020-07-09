cmake_minimum_required (VERSION 3.3)
project(PreCore)

set(PRECORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

if (CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
	if (${USE_GCOV})
		set(GCOV_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
		message("Compiling with GCOV.")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4 -Wno-invalid-offsetof ${GCOV_COMPILE_FLAGS}" CACHE STRING "" FORCE)
endif()

#Secify what sources should be built for PreCore and sets them for current target
function(set_precore_sources)
	foreach(cFile ${ARGN})
		file(GLOB_RECURSE _fndGlob_${cFile} "${PRECORE_SOURCE_DIR}/*/${cFile}.cpp")

		if (NOT EXISTS ${_fndGlob_${cFile}})
			message(WARNING "Cannot find source: ${cFile}")
		else()
			target_sources(${PROJECT_NAME} PRIVATE ${_fndGlob_${cFile}})
		endif()
	endforeach()
	link_directories(${PRECORE_SOURCE_DIR})
	target_include_directories(${PROJECT_NAME} PRIVATE ${PRECORE_SOURCE_DIR})
endfunction()

function(enable_threads)
	if (CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
		set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
		set(THREADS_PREFER_PTHREAD_FLAG TRUE)
		find_package(Threads REQUIRED)

		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}" CACHE STRING "" FORCE)
	endif()
endfunction()

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
endif()