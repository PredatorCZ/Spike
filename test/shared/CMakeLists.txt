add_library(class SHARED class.cpp)
target_link_libraries(class precore)
target_compile_definitions(class PRIVATE REF_EXPORT)
add_executable(shared_test shared.cpp)
target_link_libraries(shared_test class precore)

if(MINGW)
  set_target_properties(shared_test PROPERTIES SUFFIX ".exe")
endif()

install(
  TARGETS shared_test class
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib)
