if(NOT DEFINED RELEASEVER AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(RELEASEVER TRUE)
  set(BuildType Release)
else()
  set(RELEASEVER FALSE)
  set(BuildType Debug)
endif()

# ~~~
# build_target(
#   NAME <name of target>
#   TYPE <library type or APP>
#   SOURCES <source files> ...
#   LINKS <link targets> ...
#   DEFINITIONS ...
#   INCLUDES ...
#   LINK_DIRS ...
#   AUTHOR <author string>
#   DESCR <Product description string>
#   START_YEAR <year when the project started>
#   PIC
#   PROPERTIES ...
#   NO_PROJECT_H
#   NO_VERINFO
# )
# ~~~

function(build_target)
  cmake_parse_arguments(
    _arg "NO_PROJECT_H;NO_VERINFO;PIC" "TYPE;AUTHOR;DESCR;NAME;START_YEAR"
    "SOURCES;DEFINITIONS;LINKS;INCLUDES;LINK_DIRS;PROPERTIES" ${ARGN})

  link_directories(${_arg_LINK_DIRS})
  set(_is_python_module FALSE)
  if(_arg_TYPE STREQUAL PYMODULE)
    set(_is_python_module TRUE)
    set(_arg_TYPE SHARED)
  endif()

  if(_arg_TYPE STREQUAL APP)
    add_executable(${_arg_NAME} ${_arg_SOURCES})
  else()
    add_library(${_arg_NAME} ${_arg_TYPE} ${_arg_SOURCES})
  endif()

  target_compile_definitions(${_arg_NAME} PRIVATE ${_arg_DEFINITIONS})
  target_include_directories(${_arg_NAME} PRIVATE ${_arg_INCLUDES})
  target_link_libraries(${_arg_NAME} ${_arg_LINKS})

  if(_arg_PROPERTIES)
    set_target_properties(${_arg_NAME} PROPERTIES ${_arg_PROPERTIES})
  endif()

  get_target_property(TARGET_PREFIX ${_arg_NAME} PREFIX)

  if(NOT TARGET_PREFIX)
    set(TARGET_PREFIX)
  endif()

  get_target_property(TARGET_SUFFIX ${_arg_NAME} SUFFIX)

  if(NOT TARGET_SUFFIX)
    set(TARGET_SUFFIX)
  endif()

  get_target_property(TARGET_TYPE ${_arg_NAME} TYPE)

  if(NOT PROJECT_VERSION_PATCH)
    set(PROJECT_VERSION_PATCH 0)
  endif()

  if(NOT PROJECT_VERSION_TWEAK)
    set(PROJECT_VERSION_TWEAK 0)
  endif()

  set(TARGET_AUTHOR ${_arg_AUTHOR})
  set(TARGET_DESC ${_arg_DESCR})
  set(TARGET_PRODUCT_NAME ${_arg_NAME})

  string(TIMESTAMP _TARGET_DATE_YYYY "%Y")
  set(TARGET_COPYRIGHT "Copyright (C) ")

  if(_arg_START_YEAR)
    if(NOT ${_TARGET_DATE_YYYY} STREQUAL ${_arg_START_YEAR})
      string(APPEND TARGET_COPYRIGHT ${_arg_START_YEAR} -)
    endif()
  endif()

  string(APPEND TARGET_COPYRIGHT "${_TARGET_DATE_YYYY} ")

  if(NOT _arg_NO_VERINFO)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/verinfo.rc.tmpl
                   ${PROJECT_BINARY_DIR}/${_arg_NAME}/verinfo.rc)
    target_sources(${_arg_NAME} PRIVATE ${PROJECT_BINARY_DIR}/${_arg_NAME}/verinfo.rc)
  endif()

  if(NOT _arg_NO_PROJECT_H)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/project.h.tmpl
                   ${PROJECT_BINARY_DIR}/${_arg_NAME}/project.h)
    target_include_directories(${_arg_NAME}
                               PRIVATE ${PROJECT_BINARY_DIR}/${_arg_NAME})
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
     AND _arg_PIC)
    if(${_arg_PIC})
      target_compile_options(${_arg_NAME} PRIVATE -fPIC)
    endif()
  endif()

  if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${_arg_NAME} PRIVATE -fvisibility=hidden)
  endif()

  if(${_is_python_module})
    if(WIN32)
      set_target_properties(${_arg_NAME} PROPERTIES SUFFIX .pyd)
    endif()

    set_target_properties(
      ${_arg_NAME}
      PROPERTIES PREFIX ""
                 RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                 "${CMAKE_SOURCE_DIR}/lib"
                 RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/lib"
                 LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
    target_include_directories(${_arg_NAME} PRIVATE ${Python2_INCLUDE_DIRS})
    target_link_libraries(${_arg_NAME} ${Python2_LIBRARIES})
  endif()
endfunction()
