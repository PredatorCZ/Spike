cmake_minimum_required (VERSION 3.3)
project(PreCore)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
	set(PRECORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
else()
	set(PRECORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()

#Secify what sources should be built for PreCore and sets them for current target
function(set_precore_sources)
	foreach(cFile ${ARGN})
		file(GLOB_RECURSE _fndGlob_${cFile} "${PRECORE_SOURCE_DIR}/*${cFile}.cpp")

		if (NOT EXISTS ${_fndGlob_${cFile}})
			message(WARNING "Cannot find source: ${cFile}")
		else()
			target_sources(${PROJECT_NAME} PRIVATE ${_fndGlob_${cFile}})
		endif()
	endforeach()
	link_directories(${PRECORE_SOURCE_DIR})
	target_include_directories(${PROJECT_NAME} PRIVATE ${PRECORE_SOURCE_DIR})
endfunction()

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)

enable_testing()
add_test(PreCoreTesting test/PreCoreTesting)

endif()