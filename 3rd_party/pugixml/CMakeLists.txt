cmake_minimum_required(VERSION 3.9)
project(pugixml VERSION 1.11)
set(CMAKE_CXX_STANDARD 11)

option(PUGI_STATIC_LIB "Builds pugixml-static target." OFF)

add_library(pugixml-interface INTERFACE)
target_include_directories(pugixml-interface INTERFACE include)

if((NOT NO_OBJECTS) OR PUGI_STATIC_LIB)
  add_library(pugixml-objects OBJECT pugixml.cpp)
  target_link_libraries(pugixml-objects PUBLIC pugixml-interface)
  set_target_properties(pugixml-objects PROPERTIES POSITION_INDEPENDENT_CODE
                                                   ${OBJECTS_PID})

  list(FIND EXPOSE_SYMBOLS pugixml found_index)

  if(found_index LESS 0)
    target_compile_options(pugixml-objects PRIVATE -fvisibility=hidden)
  elseif(WIN32)
    target_compile_definitions(
      pugixml-objects
      INTERFACE PUGIXML_API=__declspec\(dllimport\)
      PRIVATE PUGIXML_API=__declspec\(dllexport\))
  endif()
endif()

if(PUGI_STATIC_LIB)
  add_library(pugixml-static STATIC)
  set_target_properties(pugixml-static PRIVATE PROPERTIES OUTPUT_NAME pugixml)
  target_link_libraries(pugixml-static pugixml-objects)
  install(TARGETS pugixml-static LIBRARY DESTINATION lib)
endif()

if(BUILD_SHARED_LIBS)
  add_library(pugixml SHARED pugixml_dll.rc pugixml.cpp)
  target_link_libraries(pugixml pugixml-interface)

  if(WIN32 OR MINGW)
    set_target_properties(pugixml PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                             PREFIX "")
  else()
    set_target_properties(pugixml PROPERTIES VERSION ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_VERSION})
  endif()

  if(WIN32)
    target_compile_definitions(
      pugixml
      INTERFACE PUGIXML_API=__declspec\(dllimport\)
      PRIVATE PUGIXML_API=__declspec\(dllexport\))
  endif()
  install(
    TARGETS pugixml
    LIBRARY NAMELINK_SKIP DESTINATION $<IF:$<BOOL:${MINGW}>,bin,lib>
    RUNTIME DESTINATION bin)
endif()
