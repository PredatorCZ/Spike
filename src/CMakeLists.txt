set(PC_SOURCES
    blowfish.cpp
    crc32.cpp
    directory_scanner.cpp
    master_printer.cpp
    matrix44.cpp
    reflector_io.cpp
    reflector_xml.cpp
    reflector.cpp
    stat.cpp
    uni.cpp
    uni_format.cpp
)

# OBJECTS
if(NOT NO_OBJECTS)
  add_library(spike-objects OBJECT ${PC_SOURCES})
  target_link_libraries(spike-objects pugixml-interface spike-interface)
  set_target_properties(spike-objects PROPERTIES POSITION_INDEPENDENT_CODE
                                                 ${OBJECTS_PID})
  target_compile_options(spike-objects PRIVATE -fvisibility=hidden)
endif()

if(BUILD_SHARED_LIBS)
  add_library(spike SHARED ${PC_SOURCES})
  target_link_libraries(spike PUBLIC spike-interface pugixml)

  target_compile_definitions(
    spike
    PRIVATE PC_EXPORT
    INTERFACE PC_IMPORT)

  target_compile_options(spike PUBLIC -fvisibility=hidden)

  if(MINGW)
    target_compile_options(spike PUBLIC -municode -fexcess-precision=fast)
    target_compile_definitions(spike PUBLIC UNICODE _UNICODE)
  endif()

  if(WIN32 OR MINGW)
    set_target_properties(spike PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                           PREFIX "")
  else()
    set_target_properties(spike PROPERTIES VERSION ${PROJECT_VERSION}
                                           SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  if(MINGW)
    target_link_libraries(spike
                          -Wl,--strip-all,--discard-all,--exclude-all-symbols)
  endif()

  install(
    TARGETS spike
    LIBRARY NAMELINK_SKIP DESTINATION $<IF:$<BOOL:${MINGW}>,bin,lib>
    RUNTIME DESTINATION bin)
endif()

if(GLTF)
  add_subdirectory(gltf)
endif()

if(APP_OBJECTS)
  add_subdirectory(app)
endif()

if(CLI)
  add_subdirectory(cli)
endif()
