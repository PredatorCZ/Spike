# A 3ds Max CMAKE Module
# Required variales
#  TARGETEX_LOCATION = a location to this module
# Optional variables
#  MAX_VERSION = set version of 3ds max
#  MaxDirectory = a directory, where "3ds max yyyy" and it's SDK are located
# Output variables
#  MaxSDK = a path to a SDK
#  MaxPlugins = a path to a "3ds max yyyy/plugins" folder
#  MaxSDKLibrariesPath = a path to a lib folder from SDK
#  MaxDefinitions = target definitions
#  MaxProperties = target properties
#  build_morpher() = building a morpher library

include(${TARGETEX_LOCATION}/targetex.cmake)

if (NOT DEFINED MAX_VERSION)
	set (MAX_VERSION 2017)
endif()

if (NOT MaxDirectory)
    set(MaxDirectory "C:/Program Files/Autodesk/3ds Max ${MAX_VERSION}")
endif()

set (MaxSDK "${MaxDirectory} SDK/maxsdk")
set (MaxPlugins "${MaxDirectory}/plugins")

if (MAX_VERSION LESS 2014)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set (_SDK_libs_path_prefix /x64)
	endif()
else()
	set (_SDK_libs_path_suffix /x64/Release)
endif()

set (MaxSDKLibrariesPath ${MaxSDK}${_SDK_libs_path_prefix}/lib${_SDK_libs_path_suffix})
set (MAX_EX_DIR ${PROJECT_SOURCE_DIR}/${TARGETEX_LOCATION}/../MAXex/)

if (MAX_VERSION GREATER 2013)
    add_definitions(-D_UNICODE -DUNICODE)
    set(CHAR_TYPE UNICODE)
else()
    set(CHAR_TYPE CHAR)
endif()

include(${TARGETEX_LOCATION}/chartype.cmake)

if (RELEASEVER EQUAL TRUE)
	set (WPO /GL) #Whole program optimalization
endif()

set(MaxDefinitions 
    #Defines
    -D_USRDLL -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -D_WIN32_WINDOWS=0x0601
    -D_WIN32_IE=0x0800 -D_WINDOWS -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE
    -D_SCL_SECURE_NO_DEPRECATE -DISOLATION_AWARE_ENABLED=1 -DBUILDVERSION=${MAX_VERSION} 
    -D_WINDLL

    #Optimalization:
    /Gy #Function level linking (COMDAT)
    /Zc:inline #Cleanup
    /Oi #Intrinsic methods
    /fp:fast #Fast floating point
    /Zc:forScope #For loop conformance
    ${WPO}

    #Checks:
    /GS #Security check
    /sdl

    #Other:
    /Zc:wchar_t #Built-in type
    /Gm- #No minimal rebuild
    /Gd #cldecl
    /errorReport:prompt #Compiler error reporting
    /nologo

    /w34996 /we4706 /we4390 /we4557 /we4546 /we4545 /we4295 /we4310
    /we4130 /we4611 /we4213 /we4121 /we4715 /w34701 /w34265 /wd4244 /wd4018 /wd4819
)
set(MaxProperties
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${MaxPlugins}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${MaxPlugins}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin/${CMAKE_GENERATOR_PLATFORM}_${MAX_VERSION}
)
function(build_morpher)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK COMMAND lib 
	    ARGS /def:"${MAX_EX_DIR}Morpher${CMAKE_GENERATOR_PLATFORM}.def" /out:Morpher.lib /machine:${CMAKE_GENERATOR_PLATFORM})
endfunction()