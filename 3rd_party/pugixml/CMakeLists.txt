cmake_minimum_required(VERSION 3.4)
project(pugixml VERSION 1.11)
set(CMAKE_CXX_STANDARD 11)

option(PUGI_STATIC_OBJECTS "Builds pugixml-objects target." OFF)
option(PUGI_SHARED_OBJECTS "Builds pugixml-shared target." OFF)
option(PUGI_STATIC_LIBS "Builds pugixml-static target." OFF)
option(PUGI_SHARED_LIBS "Builds pugixml target." OFF)

if(PUGI_STATIC_LIBS)
  set(PUGI_STATIC_OBJECTS ON)
endif()

if(PUGI_SHARED_LIBS)
  set(PUGI_SHARED_OBJECTS ON)
endif()

if(PUGI_STATIC_OBJECTS)
  add_library(pugixml-objects OBJECT pugixml)
  target_include_directories(pugixml-objects PUBLIC include)
endif()

if(PUGI_SHARED_OBJECTS)
  add_library(pugixml-shared OBJECT pugixml)
  target_include_directories(pugixml-shared PUBLIC include)
  set_target_properties(pugixml-shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(NOT MSVC AND NOT PUGI_SHARED_LIBS)
    target_compile_options(pugixml-shared PRIVATE -fvisibility=hidden)
  endif()
endif()

if(PUGI_STATIC_LIBS)
  add_library(pugixml-static STATIC)
  set_target_properties(pugixml-static PRIVATE PROPERTIES OUTPUT_NAME pugixml)
  target_link_libraries(pugixml-static pugixml-objects)
  install(TARGETS pugixml-static LIBRARY DESTINATION lib)
endif()

if(PUGI_SHARED_LIBS)
  add_library(pugixml SHARED pugixml_dll.rc)
  target_link_libraries(pugixml pugixml-shared)
  if(MSVC)
    target_compile_definitions(
      pugixml-shared
      INTERFACE PUGIXML_API=__declspec\(dllimport\)
      PRIVATE PUGIXML_API=__declspec\(dllexport\))
  else()
    set_target_properties(pugixml PROPERTIES VERSION ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()
  install(
    TARGETS pugixml
    LIBRARY NAMELINK_SKIP DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
