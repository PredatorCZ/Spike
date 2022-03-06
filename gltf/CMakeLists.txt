cmake_minimum_required(VERSION 3.4)
project(gltf VERSION 1)
set(CMAKE_CXX_STANDARD 17)

option(GLTF_STATIC_OBJECTS "Builds gltf-objects target." OFF)
option(GLTF_SHARED_OBJECTS "Builds gltf-shared target." OFF)
option(GLTF_SHARED_LIB "Builds gltf shared library." OFF)


if(GLTF_STATIC_OBJECTS)
  add_library(gltf-objects OBJECT src/gltf)
  target_include_directories(gltf-objects PUBLIC include ../3rd_party/json)
endif()

if(GLTF_SHARED_OBJECTS OR GLTF_SHARED_LIB)
  add_library(gltf-shared OBJECT src/gltf)
  target_include_directories(gltf-shared PUBLIC include ../3rd_party/json)
  set_target_properties(gltf-shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(NOT MSVC AND NOT GLTF_SHARED_LIBS)
    target_compile_options(gltf-shared PRIVATE -fvisibility=hidden)
  endif()
endif()

if (GLTF_SHARED_LIB)
  add_library(gltf SHARED)
  target_link_libraries(gltf gltf-shared)
  target_compile_definitions(gltf INTERFACE GLTF_IMPORT)
  target_compile_definitions(gltf-shared PRIVATE GLTF_EXPORT)

  if(MSVC OR MINGW)
    set_target_properties(gltf PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                          PREFIX "")
  else()
    set_target_properties(gltf PROPERTIES VERSION ${PROJECT_VERSION}
                                          SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  install(
    TARGETS gltf
    LIBRARY NAMELINK_SKIP DESTINATION $<IF:$<BOOL:${MINGW}>,bin,lib>
    RUNTIME DESTINATION bin)
endif()
