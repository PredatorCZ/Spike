if(NOT DEFINED RELEASEVER AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(RELEASEVER TRUE)
  set(BuildType Release)
else()
  set(RELEASEVER FALSE)
  set(BuildType Debug)
endif()

# ~~~
# build_target(
#   TYPE <library type or APP>
#   SOURCES <source files> ...
#   LINKS <link targets> ...
#   DEFINITIONS ...
#   INCLUDES ...
#   LINK_DIRS ...
#   AUTHOR <author string>
#   DESCR <Product description string>
#   NAME <Product name string>
#   START_YEAR <year when the project started>
#   PROPERTIES ...
#   NO_PROJECT_H
#   NO_VERINFO
# )
# ~~~

function(build_target)
  cmake_parse_arguments(
    _arg "NO_PROJECT_H;NO_VERINFO" "TYPE;AUTHOR;DESCR;NAME;START_YEAR"
    "SOURCES;DEFINITIONS;LINKS;INCLUDES;LINK_DIRS;PROPERTIES" ${ARGN})

  link_directories(${_arg_LINK_DIRS})
  set(_is_python_module FALSE)
  if(_arg_TYPE STREQUAL PYMODULE)
    set(_is_python_module TRUE)
    set(_arg_TYPE SHARED)
  endif()

  if(_arg_TYPE STREQUAL APP)
    add_executable(${PROJECT_NAME} ${_arg_SOURCES})
  else()
    add_library(${PROJECT_NAME} ${_arg_TYPE} ${_arg_SOURCES})
  endif()

  target_compile_definitions(${PROJECT_NAME} PRIVATE ${_arg_DEFINITIONS})
  target_include_directories(${PROJECT_NAME} PRIVATE ${_arg_INCLUDES})
  target_link_libraries(${PROJECT_NAME} ${_arg_LINKS})

  if(_arg_PROPERTIES)
    set_target_properties(${PROJECT_NAME} PROPERTIES ${_arg_PROPERTIES})
  endif()

  get_target_property(PROJECT_PREFIX ${PROJECT_NAME} PREFIX)

  if(NOT PROJECT_PREFIX)
    set(PROJECT_PREFIX)
  endif()

  get_target_property(PROJECT_SUFFIX ${PROJECT_NAME} SUFFIX)

  if(NOT PROJECT_SUFFIX)
    set(PROJECT_SUFFIX)
  endif()

  get_target_property(PROJECT_TYPE ${PROJECT_NAME} TYPE)

  if(NOT PROJECT_VERSION_PATCH)
    set(PROJECT_VERSION_PATCH 0)
  endif()

  if(NOT PROJECT_VERSION_TWEAK)
    set(PROJECT_VERSION_TWEAK 0)
  endif()

  set(PROJECT_AUTHOR ${_arg_AUTHOR})
  set(PROJECT_DESC ${_arg_DESCR})
  set(PROJECT_PRODUCT_NAME ${_arg_NAME})

  if(PROJECT_AUTHOR)
    string(TIMESTAMP _PROJECT_DATE_YYYY "%Y")
    set(PROJECT_COPYRIGHT "Copyright (C) ")

    if(_arg_START_YEAR)
      if(NOT ${_PROJECT_DATE_YYYY} STREQUAL ${_arg_START_YEAR})
        string(APPEND PROJECT_COPYRIGHT ${_arg_START_YEAR} -)
      endif()
    endif()

    string(APPEND PROJECT_COPYRIGHT "${_PROJECT_DATE_YYYY} ${PROJECT_AUTHOR}")
  endif()

  if(NOT _arg_NO_VERINFO)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/verinfo.rc.tmpl
                   ${PROJECT_BINARY_DIR}/verinfo.rc)
    target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR}/verinfo.rc)
  endif()

  if(NOT _arg_NO_PROJECT_H)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/project.h.tmpl
                   ${PROJECT_SOURCE_DIR}/project.h)
  endif()

  if(${_is_python_module})
    if(WIN32)
      set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX .pyd)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
      target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -fvisibility=hidden)
    endif()

    set_target_properties(
      ${PROJECT_NAME}
      PROPERTIES PREFIX ""
                 RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                 "${CMAKE_SOURCE_DIR}/lib"
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib"
                 LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Python2_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${Python2_LIBRARIES})
  endif()
endfunction()
