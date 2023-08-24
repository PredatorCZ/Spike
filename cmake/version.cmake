function(context_version VAR)
  file(GLOB_RECURSE CTX_FILES "${CMAKE_SOURCE_DIR}/*/app_context.hpp")
  set(LAST_LEN 0xffffff)

  foreach(CTX_FILE ${CTX_FILES})
    string(LENGTH ${CTX_FILE} THIS_LEN)
    if(${THIS_LEN} LESS ${LAST_LEN})
      set(LAST_LEN ${THIS_LEN})
      set(CTX_REGX "CONTEXT_VERSION = ([0-9]+);")
      file(STRINGS ${CTX_FILE} CTX_VAR REGEX ${CTX_REGX})
      string(REGEX MATCH ${CTX_REGX} CTX_VERSION ${CTX_VAR})
      set(${VAR} ${CMAKE_MATCH_1} PARENT_SCOPE)
    endif()
  endforeach()
endfunction()

find_package(Git REQUIRED)

function(toolset_version)
  if(NOT SK_VERSION_MINOR)
    context_version(SK_VERSION_MINOR)
    set(SK_VERSION_MINOR ${SK_VERSION_MINOR} CACHE STRING "")
  endif()

  execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 --match=toolset-v[0-9]* OUTPUT_VARIABLE LAST_TAG)

  if(NOT LAST_TAG)
    execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0 --match=v[0-9]* OUTPUT_VARIABLE LAST_TAG)
  endif()

  string(STRIP "${LAST_TAG}" LAST_TAG)
  set(TAG_RANGE "${LAST_TAG}..")

  if(NOT LAST_TAG)
    set(LAST_TAG "v0.0.0")
    set(TAG_RANGE "HEAD")
  endif()

  execute_process(COMMAND ${GIT_EXECUTABLE} rev-list "${TAG_RANGE}" --count OUTPUT_VARIABLE NUM_COMMITS)
  string(REGEX MATCH "v([0-9]+).[0-9]+.?([0-9]+)?" GIT_VERSION ${LAST_TAG})
  math(EXPR PROJECT_VERSION_PATCH_ ${CMAKE_MATCH_2}+${NUM_COMMITS})
  set(PROJECT_VERSION_ "${CMAKE_MATCH_1}.${SK_VERSION_MINOR}.${PROJECT_VERSION_PATCH_}")

  set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1} PARENT_SCOPE)
  set(PROJECT_VERSION_MINOR ${SK_VERSION_MINOR} PARENT_SCOPE)
  set(PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH_} PARENT_SCOPE)
  set(PROJECT_VERSION "${PROJECT_VERSION_}" PARENT_SCOPE)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version "${PROJECT_VERSION_}")

  message("-- ${PROJECT_NAME} version: ${PROJECT_VERSION_}")
endfunction()

function(num_commits FILE_PATH IOVAL)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} log --pretty=format:commit:%H --follow -- ${FILE_PATH}
    OUTPUT_VARIABLE COMMITS
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  if(NOT COMMITS)
    execute_process(
      COMMAND  ${GIT_EXECUTABLE} submodule foreach git log --pretty=format:commit:%H --follow -- ${FILE_PATH}
      OUTPUT_VARIABLE COMMITS
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  string(REGEX MATCHALL "commit:[0-9a-f]+" RE_COMMITS ${COMMITS})
  set(COMMIT_HASHES ${${IOVAL}})
  list(LENGTH RE_COMMITS RE_COMMITS_LEN)

  foreach(COMMIT ${RE_COMMITS})
    list(FIND COMMIT_HASHES ${COMMIT} FOUND_COMMIT)

    if(FOUND_COMMIT LESS 0)
      list(APPEND COMMIT_HASHES ${COMMIT})
    endif()
  endforeach()

  set(${IOVAL} ${COMMIT_HASHES} PARENT_SCOPE)
endfunction()

function(target_version TARGET MAJOR_VERSION)
  cmake_parse_arguments(_arg "TO_FILE;SKMODULE" "" "" ${ARGN})

  get_target_property(TARGET_SOURCES ${TARGET} SOURCES)
  file(GLOB_RECURSE PROJECT_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")
  set(VALID_SOURCES)
  set(EXTERNAL_SOURCES)

  foreach(source_rel ${TARGET_SOURCES})
    cmake_path(IS_ABSOLUTE source_rel source_is_absolute)
    set(source_abs ${source_rel})

    if(source_is_absolute)
      cmake_path(RELATIVE_PATH source_rel BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE source_abs)
    else()
      cmake_path(ABSOLUTE_PATH source_abs BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" OUTPUT_VARIABLE source_rel)
    endif()

    set(FOUND_SOURCE)

    foreach(psource ${PROJECT_SOURCES})
      if(${psource} STREQUAL ${source_abs})
        list(APPEND VALID_SOURCES ${source_abs})
        set(FOUND_SOURCE TRUE)
        break()
      endif()
    endforeach()

    if(NOT FOUND_SOURCE)
      list(APPEND EXTERNAL_SOURCES ${source_rel})
    endif()
  endforeach()

  set(COMMIT_HASHES "")
  set(EXTERNAL_COMMIT_HASHES "")

  foreach(V ${VALID_SOURCES})
    num_commits(${V} COMMIT_HASHES)
  endforeach()

  foreach(E ${EXTERNAL_SOURCES})
    num_commits(${E} EXTERNAL_COMMIT_HASHES)
  endforeach()

  list(LENGTH COMMIT_HASHES COMMIT_HASHES_LEN)
  list(LENGTH EXTERNAL_COMMIT_HASHES EXTERNAL_COMMIT_HASHES_LEN)

  if(NOT SK_VERSION_MINOR)
    context_version(SK_VERSION_MINOR)
    set(SK_VERSION_MINOR ${SK_VERSION_MINOR} CACHE STRING "")
  endif()

  if(NOT _arg_SKMODULE)
    set(PROJECT_VERSION_ "${MAJOR_VERSION}.${COMMIT_HASHES_LEN}")
    set(PROJECT_VERSION_MINOR ${COMMIT_HASHES_LEN} PARENT_SCOPE)

    if(EXTERNAL_COMMIT_HASHES_LEN GREATER 0)
      set(PROJECT_VERSION_PATCH ${EXTERNAL_COMMIT_HASHES_LEN} PARENT_SCOPE)
      set(PROJECT_VERSION_ "${PROJECT_VERSION_}.${EXTERNAL_COMMIT_HASHES_LEN}")
    endif()
  else()
    set(PROJECT_VERSION_ "${MAJOR_VERSION}.${SK_VERSION_MINOR}.${COMMIT_HASHES_LEN}")
    set(PROJECT_VERSION_MINOR ${SK_VERSION_MINOR} PARENT_SCOPE)
    set(PROJECT_VERSION_PATCH ${COMMIT_HASHES_LEN} PARENT_SCOPE)
  endif()

  set(PROJECT_VERSION_MAJOR ${MAJOR_VERSION} PARENT_SCOPE)
  set(PROJECT_VERSION "${PROJECT_VERSION_}" PARENT_SCOPE)

  if(_arg_TO_FILE)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/version_${TARGET} "${PROJECT_VERSION_}")
  endif()

  message("-- ${PROJECT_NAME} version: ${PROJECT_VERSION_}")
endfunction()
