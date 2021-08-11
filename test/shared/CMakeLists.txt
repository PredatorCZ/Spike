add_library(class SHARED class.cpp)
target_link_libraries(class precore)
add_executable(shared_test shared.cpp)
target_link_libraries(shared_test class precore)
add_test(shared_test shared_test)

if(MINGW)
  set_target_properties(shared_test PROPERTIES SUFFIX ".exe")
  install(
    TARGETS shared_test class
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin)
endif()
