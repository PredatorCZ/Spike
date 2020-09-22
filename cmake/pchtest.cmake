
# Tests headers, if they are independent by creating precompiled headers
function(independence_check)
  if(CMAKE_CXX_COMPILER_ID MATCHES Clang OR CMAKE_COMPILER_IS_GNUCXX)
    cmake_parse_arguments(_arg "" "" "ARGS;PATHS" ${ARGN})
    list(APPEND CORE_SOURCE_FILES)

    foreach(cFile ${_arg_PATHS})
      file(GLOB_RECURSE files "${CMAKE_CURRENT_SOURCE_DIR}/${cFile}/*.hpp")
      foreach(f ${files})
        list(APPEND CORE_SOURCE_FILES ${f})
      endforeach()
    endforeach()

    foreach(cArg ${_arg_ARGS})
      list(APPEND CORE_SOURCE_FILES ${cArg})
    endforeach()

    add_test(NAME ${PROJECT_NAME}_ITest COMMAND ${CMAKE_CXX_COMPILER}
                                                ${CORE_SOURCE_FILES} -Wno-pragma-once-outside-header)

    add_test(
      NAME ${PROJECT_NAME}_ITest_Cleanup
      COMMAND find -type f -name *.gch -delete
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    set_tests_properties(${PROJECT_NAME}_ITest_Cleanup
                         PROPERTIES DEPENDS ${PROJECT_NAME}_ITest)
  endif()
endfunction()
