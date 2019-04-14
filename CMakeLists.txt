cmake_minimum_required (VERSION 3.3)


add_executable (PreCoreTesting "_samples/datas.cpp" "datas/reflector.cpp" "datas/MasterPrinter.cpp" "datas/esString.cpp")
include_directories("./")

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -pthread")
endif()
