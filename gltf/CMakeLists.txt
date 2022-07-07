cmake_minimum_required(VERSION 3.4)
project(gltf VERSION 1)
set(CMAKE_CXX_STANDARD 20)

add_library(gltf-interface INTERFACE)
target_include_directories(gltf-interface INTERFACE include ../3rd_party/json)

if(NOT NO_OBJECTS)
  add_library(gltf-objects OBJECT src/gltf.cpp src/fx_gltf.cpp)
  target_link_libraries(gltf-objects PUBLIC gltf-interface precore-objects)
  set_target_properties(gltf-objects PROPERTIES POSITION_INDEPENDENT_CODE
                                                   ${OBJECTS_PID})
  target_compile_options(gltf-objects PRIVATE -fvisibility=hidden)
endif()

if (BUILD_SHARED_LIBS)
  add_library(gltf SHARED src/gltf.cpp src/fx_gltf.cpp)
  target_link_libraries(gltf gltf-interface precore)
  target_compile_definitions(gltf INTERFACE GLTF_IMPORT PRIVATE GLTF_EXPORT)

  if(WIN32 OR MINGW)
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
