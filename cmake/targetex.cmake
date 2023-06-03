if(NOT DEFINED RELEASEVER AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(RELEASEVER TRUE)
else()
  set(RELEASEVER FALSE)
endif()

if(MINGW)
  set(CMAKE_RC_COMPILER
      "x86_64-w64-mingw32-windres"
      CACHE STRING "" FORCE)
  set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>"
      CACHE STRING "" FORCE)
  enable_language(RC)
endif()

if(WIN32)
  add_definitions(-D_UNICODE -DUNICODE)
endif()

# ~~~
# build_target(
#   NAME <name of target>
#   TYPE <library type or APP or PYMODULE>
#   SOURCES <source files> ...
#   LINKS <link targets> ...
#   DEFINITIONS ...
#   INCLUDES ...
#   LINK_DIRS ...
#   AUTHOR <author string>
#   DESCR <Product description string>
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
  set(_is_es_module FALSE)
  if(_arg_TYPE STREQUAL PYMODULE)
    set(_is_python_module TRUE)
    set(_arg_TYPE SHARED)
  endif()

  if(_arg_TYPE STREQUAL ESMODULE)
    set(_is_es_module TRUE)
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

  get_target_property(TARGET_TYPE ${_arg_NAME} TYPE)

  set(PROJECT_VERSION_MAJOR_ 0)
  set(PROJECT_VERSION_MINOR_ 0)
  set(PROJECT_VERSION_PATCH_ 0)
  set(PROJECT_VERSION_TWEAK_ 0)

  if(DEFINED PROJECT_VERSION)
    set(PROJECT_VERSION_ ${PROJECT_VERSION})

    if(PROJECT_VERSION_MAJOR GREATER 0)
      set(PROJECT_VERSION_MAJOR_ ${PROJECT_VERSION_MAJOR})
    endif()

    if(PROJECT_VERSION_MINOR GREATER 0)
      set(PROJECT_VERSION_MINOR_ ${PROJECT_VERSION_MINOR})
    endif()

    if(PROJECT_VERSION_PATCH GREATER 0)
      set(PROJECT_VERSION_PATCH_ ${PROJECT_VERSION_PATCH})
    endif()

    if(PROJECT_VERSION_TWEAK GREATER 0)
      set(PROJECT_VERSION_TWEAK_ ${PROJECT_VERSION_TWEAK})
    endif()
  else()
    set(PROJECT_VERSION_ no_version)
  endif()

  if(MINGW)
    if(_arg_TYPE STREQUAL APP)
      set_target_properties(${_arg_NAME} PROPERTIES SUFFIX ".exe")
    elseif(_arg_TYPE STREQUAL SHARED)
      set_target_properties(${_arg_NAME}
                            PROPERTIES SUFFIX "${PROJECT_VERSION_MAJOR_}.dll")
    endif()
  endif()

  get_target_property(TARGET_PREFIX ${_arg_NAME} PREFIX)

  if(NOT TARGET_PREFIX)
    if(_arg_TYPE STREQUAL SHARED)
      set(TARGET_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
    else()
      set(TARGET_PREFIX)
    endif()
  endif()

  get_target_property(TARGET_SUFFIX ${_arg_NAME} SUFFIX)

  if(NOT TARGET_SUFFIX)
    if(_arg_TYPE STREQUAL SHARED)
      set(TARGET_PREFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
      set(TARGET_SUFFIX)
    endif()
    set(TARGET_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()

  if(_arg_TYPE STREQUAL SHARED)
    set_target_properties(${_arg_NAME} PROPERTIES VERSION
                                                  ${PROJECT_VERSION_MAJOR_})
  endif()

  set(TARGET_AUTHOR ${_arg_AUTHOR})
  set(TARGET_DESC ${_arg_DESCR})
  set(TARGET_NAME ${_arg_NAME})
  set(TARGET_PRODUCT_NAME ${PROJECT_NAME})

  string(TIMESTAMP _TARGET_DATE_YYYY "%Y")
  set(TARGET_COPYRIGHT "Copyright (C) ")

  if(_arg_START_YEAR)
    if(NOT ${_TARGET_DATE_YYYY} STREQUAL ${_arg_START_YEAR})
      string(APPEND TARGET_COPYRIGHT ${_arg_START_YEAR} -)
    endif()
  endif()

  string(APPEND TARGET_COPYRIGHT "${_TARGET_DATE_YYYY} ")

  if(NOT _arg_NO_VERINFO AND CMAKE_RC_COMPILER)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/verinfo.rc.tmpl
                   ${PROJECT_BINARY_DIR}/${_arg_NAME}_/verinfo.rc)
    target_sources(${_arg_NAME}
                   PRIVATE ${PROJECT_BINARY_DIR}/${_arg_NAME}_/verinfo.rc)
  endif()

  if(NOT _arg_NO_PROJECT_H)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/project.h.tmpl
                   ${PROJECT_BINARY_DIR}/${_arg_NAME}_/project.h)
    target_include_directories(${_arg_NAME}
                               PRIVATE ${PROJECT_BINARY_DIR}/${_arg_NAME}_)
  endif()

  if(NOT MSVC)
    target_compile_options(${_arg_NAME} PRIVATE -fvisibility=hidden)
  endif()

  if(${_is_es_module})
    set(module_suffix_ .${PROJECT_VERSION_MAJOR_})

    if(PROJECT_VERSION_TWEAK_ GREATER 0)
      string(
        APPEND
        module_suffix_
        .${PROJECT_VERSION_MINOR_}.${PROJECT_VERSION_PATCH_}.${PROJECT_VERSION_TWEAK_}
      )
    elseif(PROJECT_VERSION_PATCH_ GREATER 0)
      string(APPEND module_suffix_
             .${PROJECT_VERSION_MINOR_}.${PROJECT_VERSION_PATCH_})
    elseif(PROJECT_VERSION_MINOR_ GREATER 0)
      string(APPEND module_suffix_ .${PROJECT_VERSION_MINOR_})
    endif()

    set_target_properties(
      ${_arg_NAME}
      PROPERTIES SUFFIX ${module_suffix_}.spk
                 PREFIX ""
                 NO_SONAME TRUE)

    if(WIN32 OR MINGW)
      get_target_property(spike_exec spike OUTPUT_NAME)
      configure_file(${PRECORE_SOURCE_DIR}/cmake/spike.tmpl
                     ${PROJECT_BINARY_DIR}/${_arg_NAME}_/spike.runner)

      install(
        PROGRAMS ${PROJECT_BINARY_DIR}/${_arg_NAME}_/spike.runner
        TYPE BIN
        RENAME ${_arg_NAME}.cmd)
    endif()

    install(
      TARGETS ${_arg_NAME}
      LIBRARY DESTINATION $<IF:$<BOOL:${UNIX}>,bin,lib>
      RUNTIME DESTINATION bin)
  endif()

  if(${_is_python_module})
    if(WIN32 OR MINGW)
      set_target_properties(${_arg_NAME} PROPERTIES SUFFIX .pyd)
    endif()

    set_target_properties(${_arg_NAME} PROPERTIES PREFIX "")
    target_include_directories(${_arg_NAME} PRIVATE ${Python3_INCLUDE_DIRS})
    target_link_libraries(${_arg_NAME} ${Python3_LIBRARIES})
  endif()
endfunction()
