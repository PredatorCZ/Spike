function(test_odr)
  cmake_parse_arguments(_arg "" "" "EXCLUDE;PATHS" ${ARGN})
  list(APPEND CORE_SOURCE_FILES)

  foreach(cFile ${_arg_PATHS})
    file(GLOB_RECURSE files "${cFile}/*.hpp")
    foreach(f ${files})
      list(APPEND CORE_SOURCE_FILES ${f})
    endforeach()
  endforeach()

  foreach(cPath ${_arg_EXCLUDE})
    list(FILTER CORE_SOURCE_FILES EXCLUDE REGEX ${cPath})
  endforeach()

  foreach(cPath ${_arg_PATHS})
    list(TRANSFORM CORE_SOURCE_FILES REPLACE "${cPath}[/\\]?" "")
  endforeach()

  foreach(header_file ${CORE_SOURCE_FILES})
    string(REGEX REPLACE "[/\\]" "-" source_file ${header_file})
    get_filename_component(source_file ${source_file} NAME_WE)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/odr.tmpl
                   ${CMAKE_CURRENT_BINARY_DIR}/test_odr/${source_file}.1.odr.cpp)
    configure_file(${PRECORE_SOURCE_DIR}/cmake/odr.tmpl
                   ${CMAKE_CURRENT_BINARY_DIR}/test_odr/${source_file}.2.odr.cpp)
  endforeach()

  file(GLOB_RECURSE ODR_SOURCE_FILES "${CMAKE_CURRENT_BINARY_DIR}/test_odr/*.cpp")
  add_library(test_odr SHARED ${ODR_SOURCE_FILES})
  target_include_directories(test_odr PRIVATE ${_arg_PATHS})
endfunction()
