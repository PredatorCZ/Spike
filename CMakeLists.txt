cmake_minimum_required(VERSION 3.3)
project(PreCore)

set(PRECORE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE STRING "")

set(_is_cool_compiler
    FALSE
    CACHE BOOL "")
if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
  set(_is_cool_compiler
      TRUE
      CACHE BOOL "" FORCE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
  message("Compiling with GCOV.")
endif()

function(_set_basic_flags TARGET_NAME)
  if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
    target_compile_options(${TARGET_NAME} PRIVATE -fprofile-arcs
                                                  -ftest-coverage)
    target_link_libraries(${TARGET_NAME} gcov)
  endif()

  if(${_is_cool_compiler})
    target_compile_options(${TARGET_NAME} PRIVATE -msse4 -Wno-invalid-offsetof)
  endif()
endfunction()

function(_set_threads TARGET_NAME)
  if(${_is_cool_compiler})
    set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} Threads::Threads)
  endif()
endfunction()

# Secify what sources should be built for PreCore and sets them for current
# target
function(set_precore_sources TARGET_NAME)
  _set_basic_flags(${TARGET_NAME})
  target_include_directories(${TARGET_NAME} PRIVATE ${PRECORE_SOURCE_DIR})

  foreach(cFile ${ARGN})
    if(${cFile} STREQUAL multi_thread)
      _set_threads(${TARGET_NAME})
      continue()
    endif()
    file(GLOB_RECURSE _fndGlob_${cFile} "${PRECORE_SOURCE_DIR}/*/${cFile}.cpp")

    if(NOT EXISTS ${_fndGlob_${cFile}})
      message(WARNING "Cannot find source: ${cFile}.cpp")
    else()
      target_sources(${TARGET_NAME} PRIVATE ${_fndGlob_${cFile}})
    endif()
  endforeach()
endfunction()

#if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  enable_testing()
  add_subdirectory(test)
#endif()
