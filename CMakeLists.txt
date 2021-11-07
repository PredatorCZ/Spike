cmake_minimum_required(VERSION 3.12)
project(PreCore VERSION 6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(PRECORE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE STRING "")
set(CMAKE_MODULE_PATH
    ${PRECORE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}
    CACHE STRING "")
option(PC_SHARED_LIB "" OFF)
option(PRECORE_AS_SYSTEM "" OFF)

if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
  message("Compiling with GCOV.")
endif()

add_subdirectory(3rd_party/glm)

function(_set_basic_flags TARGET_NAME SCOPE)
  if(CMAKE_COMPILER_IS_GNUCXX AND USE_GCOV)
    target_compile_options(${TARGET_NAME} ${SCOPE} -fprofile-arcs
                           -ftest-coverage)
    target_link_libraries(${TARGET_NAME} gcov)
  endif()

  if(NOT MSVC)
    target_compile_options(${TARGET_NAME} ${SCOPE} -msse4 -Wno-invalid-offsetof)
  else()
    target_compile_definitions(${TARGET_NAME} ${SCOPE} _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
  endif()
endfunction()

function(_set_threads TARGET_NAME)
  if(NOT (MSVC OR MINGW))
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(${TARGET_NAME} Threads::Threads)
  endif()
endfunction()

function(_set_no_rtti TARGET_NAME SCOPE)
  if(MSVC)
    target_compile_options(${TARGET_NAME} ${SCOPE} /GR-)
  else()
    target_compile_options(${TARGET_NAME} ${SCOPE} -fno-rtti)
  endif()
endfunction()

# Secify what sources should be built for PreCore and sets them for current
# target
function(set_precore_sources TARGET_NAME)
  _set_basic_flags(${TARGET_NAME} PRIVATE)
  if(PRECORE_AS_SYSTEM)
    target_include_directories(${TARGET_NAME} SYSTEM
                               PRIVATE ${PRECORE_SOURCE_DIR})
  else()
    target_include_directories(${TARGET_NAME} PRIVATE ${PRECORE_SOURCE_DIR})
  endif()

  foreach(cFile ${ARGN})
    if(${cFile} STREQUAL multi_thread)
      _set_threads(${TARGET_NAME})
      continue()
    elseif(${cFile} STREQUAL no_rtti)
      _set_no_rtti(${TARGET_NAME} PRIVATE)
      continue()
    endif()
    file(GLOB_RECURSE _fndGlob_${cFile} "${PRECORE_SOURCE_DIR}/*/${cFile}.cpp")

    if(NOT EXISTS ${_fndGlob_${cFile}})
      message(WARNING "Cannot find source: ${cFile}.cpp")
    else()
      target_sources(${TARGET_NAME} PRIVATE ${_fndGlob_${cFile}})
    endif()
  endforeach()
endfunction()

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(test)
  enable_testing()
  add_subdirectory(bin ${CMAKE_SOURCE_DIR}/bin)
endif()

if(PC_SHARED_LIB)
  add_library(
    precore SHARED
    datas/encrypt/blowfish
    datas/crc32
    datas/directory_scanner
    datas/master_printer
    datas/matrix44
    datas/reflector_io
    datas/reflector_xml
    datas/reflector
    datas/stat
    uni/uni
    uni/uni_format)

  target_compile_definitions(
    precore
    PRIVATE PC_EXPORT
    INTERFACE PC_IMPORT)

  _set_basic_flags(precore PUBLIC)
  _set_threads(precore)
  _set_no_rtti(precore PUBLIC)

  if(MSVC OR MINGW)
    set_target_properties(precore PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                             PREFIX "")
  else()
    set_target_properties(precore PROPERTIES VERSION ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  if(NOT MSVC)
    target_compile_options(precore PUBLIC -fvisibility=hidden)
  endif()

  if(MINGW)
    target_link_libraries(precore
                          -Wl,--strip-all,--discard-all,--exclude-all-symbols)
    target_compile_options(precore PUBLIC -municode -fexcess-precision=fast)
    target_compile_definitions(precore PUBLIC UNICODE _UNICODE)
  endif()

  target_include_directories(precore PUBLIC ${PRECORE_SOURCE_DIR})

  if(PUGI_SHARED_LIBS)
    target_link_libraries(precore pugixml)
  else()
    set(PUGI_SHARED_OBJECTS
        ON
        CACHE BOOL "")
    target_link_libraries(precore pugixml-shared)
  endif()

  target_link_libraries(precore glm::glm)

  install(
    TARGETS precore
    LIBRARY NAMELINK_SKIP DESTINATION lib
    RUNTIME DESTINATION bin)
endif()

add_subdirectory(3rd_party/pugixml)

function(install_mingw_lib libname)
  file(GLOB foundfile /usr/lib/gcc/x86_64-w64-mingw32/*-posix/${libname})
  if(NOT foundfile)
    file(GLOB foundfile /usr/x86_64-w64-mingw32/lib/${libname})
  endif()
  add_custom_target(
    ${libname}-install ALL
    COMMAND x86_64-w64-mingw32-strip --discard-all --strip-all -o
            ${CMAKE_INSTALL_PREFIX}/bin/${libname} ${foundfile})
endfunction(install_mingw_lib)

if(MINGW)
  file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin/)
  install_mingw_lib(libstdc++-6.dll)
  install_mingw_lib(libgcc_s_seh-1.dll)
  install_mingw_lib(libwinpthread-1.dll)
endif()
