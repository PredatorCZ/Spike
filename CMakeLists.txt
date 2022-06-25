cmake_minimum_required(VERSION 3.12)
project(PreCore VERSION 6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    message(FATAL_ERROR "${PROJECT_NAME} requires at least gcc-7.")
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
    message(FATAL_ERROR "${PROJECT_NAME} requires at least clang-5.0.")
  endif()
endif()

set(PRECORE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE STRING "")
set(CMAKE_MODULE_PATH
    ${PRECORE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}
    CACHE STRING "")

if(USE_GCOV)
  message("Compiling with GCOV.")
endif()

# TESTING
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(test)
  enable_testing()
  add_subdirectory(bin ${CMAKE_SOURCE_DIR}/bin)
endif()

add_subdirectory(3rd_party/glm)
add_subdirectory(3rd_party/pugixml)

# INTERFACE
add_library(precore-interface INTERFACE)
target_include_directories(precore-interface INTERFACE ${PRECORE_SOURCE_DIR})
target_link_libraries(precore-interface INTERFACE glm::glm)

if(USE_GCOV)
  target_link_options(precore-interface INTERFACE --coverage)
  if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU")
    target_compile_options(precore-interface INTERFACE -fprofile-arcs
                                                       -ftest-coverage)
  elseif(${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
    target_compile_options(precore-interface INTERFACE -fprofile-instr-generate
                                                       -fcoverage-mapping)
  endif()

endif()

target_compile_options(precore-interface INTERFACE -msse4 -Wno-invalid-offsetof
                                                   -fno-rtti)
if(WIN32)
  target_compile_definitions(
    precore-interface
    INTERFACE _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
endif()

if(NOT WIN32 OR MINGW)
  set(THREADS_PREFER_PTHREAD_FLAG TRUE)
  find_package(Threads REQUIRED)
  target_link_libraries(precore-interface INTERFACE Threads::Threads)
endif()

set(PC_SOURCES
    datas/encrypt/blowfish.cpp
    datas/crc32.cpp
    datas/directory_scanner.cpp
    datas/master_printer.cpp
    datas/matrix44.cpp
    datas/reflector_io.cpp
    datas/reflector_xml.cpp
    datas/reflector.cpp
    datas/stat.cpp
    uni/uni.cpp
    uni/uni_format.cpp)

# OBJECTS
if(NOT NO_OBJECTS)
  add_library(precore-objects OBJECT ${PC_SOURCES})
  target_link_libraries(precore-objects pugixml-interface precore-interface)
  set_target_properties(precore-objects PROPERTIES POSITION_INDEPENDENT_CODE
                                                   ${OBJECTS_PID})
  target_compile_options(precore-objects PRIVATE -fvisibility=hidden)
endif()

if(BUILD_SHARED_LIBS)
  add_library(precore SHARED ${PC_SOURCES})
  target_link_libraries(precore PUBLIC precore-interface pugixml)

  target_compile_definitions(
    precore
    PRIVATE PC_EXPORT
    INTERFACE PC_IMPORT)

  target_compile_options(precore PUBLIC -fvisibility=hidden)

  if(MINGW)
    target_compile_options(precore PUBLIC -municode -fexcess-precision=fast)
    target_compile_definitions(precore PUBLIC UNICODE _UNICODE)
  endif()

  if(WIN32 OR MINGW)
    set_target_properties(precore PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                             PREFIX "")
  else()
    set_target_properties(precore PROPERTIES VERSION ${PROJECT_VERSION}
                                             SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  if(MINGW)
    target_link_libraries(precore
                          -Wl,--strip-all,--discard-all,--exclude-all-symbols)
  endif()

  install(
    TARGETS precore
    LIBRARY NAMELINK_SKIP DESTINATION $<IF:$<BOOL:${MINGW}>,bin,lib>
    RUNTIME DESTINATION bin)
endif()

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
