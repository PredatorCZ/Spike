cmake_minimum_required(VERSION 3.12)
project(PreCore VERSION 6)

set(CMAKE_CXX_STANDARD 14)

set(PRECORE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE STRING "")
set(CMAKE_MODULE_PATH
    ${PRECORE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}
    CACHE STRING "")
option(PC_SHARED_LIB "" OFF)

if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
  message("Compiling with GCOV.")
endif()

function(_set_basic_flags TARGET_NAME SCOPE)
  if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
    target_compile_options(${TARGET_NAME} ${SCOPE} -fprofile-arcs
                           -ftest-coverage)
    target_link_libraries(${TARGET_NAME} gcov)
  endif()

  if(NOT MSVC)
    target_compile_options(${TARGET_NAME} ${SCOPE} -msse4 -Wno-invalid-offsetof)
  endif()
endfunction()

function(_set_threads TARGET_NAME)
  if(NOT MSVC)
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} Threads::Threads)
  endif()
endfunction()

function(_set_no_rtti TARGET_NAME)
  if(MSVC)
    target_compile_options(${TARGET_NAME} PRIVATE /GR-)
  else()
    target_compile_options(${TARGET_NAME} PRIVATE -fno-rtti)
  endif()
endfunction()

# Secify what sources should be built for PreCore and sets them for current
# target
function(set_precore_sources TARGET_NAME)
  _set_basic_flags(${TARGET_NAME} PRIVATE)
  target_include_directories(${TARGET_NAME} PRIVATE ${PRECORE_SOURCE_DIR})

  foreach(cFile ${ARGN})
    if(${cFile} STREQUAL multi_thread)
      _set_threads(${TARGET_NAME})
      continue()
    elseif(${cFile} STREQUAL no_rtti)
      _set_no_rtti(${TARGET_NAME})
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

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  enable_testing()
  add_subdirectory(test)
endif()

if(PC_SHARED_LIB)
  add_library(
    precore SHARED
    datas/encrypt/blowfish
    datas/directory_scanner
    datas/master_printer
    datas/matrix44
    datas/reflector_io
    datas/reflector_xml
    datas/reflector
    datas/stat
    uni/uni_format)

  target_compile_definitions(
    precore
    PRIVATE PC_EXPORT
    INTERFACE PC_IMPORT)

  _set_basic_flags(precore PUBLIC)
  _set_threads(precore)

  if(NOT MSVC)
    target_compile_options(precore PRIVATE -fvisibility=hidden)
  else()
    set_target_properties(precore PROPERTIES SUFFIX ${PROJECT_VERSION}.dll)
  endif()

  target_include_directories(precore PUBLIC ${PRECORE_SOURCE_DIR})
  set_target_properties(precore PROPERTIES VERSION ${PROJECT_VERSION}
                                           SOVERSION ${PROJECT_VERSION_MAJOR})

  if(PUGI_SHARED_LIBS)
    target_link_libraries(precore pugixml)
  else()
    set(PUGI_SHARED_OBJECTS
        ON
        CACHE BOOL "")
    target_link_libraries(precore pugixml-shared)
  endif()

  install(TARGETS precore LIBRARY NAMELINK_SKIP DESTINATION bin)
endif()

add_subdirectory(3rd_party/pugixml)
