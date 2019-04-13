cmake_minimum_required (VERSION 3.8)


add_executable (PreCoreTesting "_samples/datas.cpp" "datas/reflector.cpp" "datas/MasterPrinter.cpp" "datas/esString.cpp")
include_directories("./")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -pthread")
