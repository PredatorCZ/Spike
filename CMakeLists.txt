cmake_minimum_required(VERSION 3.12)
project(Spike VERSION 7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
    message(FATAL_ERROR "${PROJECT_NAME} requires at least gcc-10.")
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)
    message(FATAL_ERROR "${PROJECT_NAME} requires at least clang-10.")
  endif()
endif()

set(SPIKE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE STRING "")
set(CMAKE_MODULE_PATH
    ${SPIKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}
    CACHE STRING "")
set(OBJECTS_PID
    OFF
    CACHE BOOL "")
set(CLI
    OFF
    CACHE BOOL "")
set(APP_OBJECTS
    OFF
    CACHE BOOL "")
set(GLTF
    OFF
    CACHE BOOL "")

if(CLI)
  set(APP_OBJECTS TRUE)
endif()

function(target_expose_defs target export_def import_def)
  list(FIND EXPOSE_SYMBOLS ${target} found_index)

  if(found_index GREATER -1)
    target_compile_definitions(${target}-interface INTERFACE ${import_def})
    target_compile_definitions(${target}-objects PRIVATE ${export_def})
  endif()
endfunction()


if(USE_GCOV)
  message("Compiling with GCOV.")
endif()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(SPIKE_STANDALONE TRUE)
else()
  set(SPIKE_STANDALONE FALSE PARENT_SCOPE)
endif()

add_subdirectory(3rd_party/glm)

# INTERFACE
add_library(spike-interface INTERFACE)
target_include_directories(spike-interface INTERFACE include)
target_link_libraries(spike-interface INTERFACE glm::glm)
target_compile_definitions(
  glm INTERFACE GLM_FORCE_DEFAULT_ALIGNED_GENTYPES GLM_FORCE_INTRINSICS
                GLM_FORCE_EXPLICIT_CTOR)

if(USE_GCOV)
  target_link_options(spike-interface INTERFACE --coverage)
  if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(spike-interface INTERFACE -fprofile-arcs
                                                     -ftest-coverage)
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(spike-interface INTERFACE -fprofile-instr-generate
                                                     -fcoverage-mapping)
  endif()

endif()

target_compile_options(spike-interface INTERFACE -msse4 -Wno-invalid-offsetof
                                                 -fno-rtti)
if(WIN32)
  target_compile_definitions(
    spike-interface
    INTERFACE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
    # Increase stack size from 1MB to 8MB
    target_link_options(spike-interface INTERFACE -Wl,/stack:8388608)
endif()

if(NOT WIN32 OR MINGW)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(spike-interface INTERFACE Threads::Threads)
endif()

# TESTING
if(SPIKE_STANDALONE)
  add_subdirectory(test)
  enable_testing()
  add_subdirectory(bin ${CMAKE_SOURCE_DIR}/bin)
endif()

add_subdirectory(3rd_party/pugixml)
add_subdirectory(src)

function(install_mingw_lib libname)
  file(GLOB foundfile /usr/lib/gcc/x86_64-w64-mingw32/*-posix/${libname})
  if(NOT foundfile)
    file(GLOB foundfile /usr/x86_64-w64-mingw32/lib/${libname})
  endif()
  add_custom_target(
    ${libname}-install ALL
    COMMAND x86_64-w64-mingw32-strip --discard-all --strip-all -o
            ${CMAKE_INSTALL_PREFIX}/bin/${libname} ${foundfile})
endfunction(install_mingw_lib)

if(MINGW)
  file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/)
  install_mingw_lib(libstdc++-6.dll)
  install_mingw_lib(libgcc_s_seh-1.dll)
  install_mingw_lib(libwinpthread-1.dll)
endif()
