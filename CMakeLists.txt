cmake_minimum_required(VERSION 2.8)

set (PROJECT_NAME "LuaState")
project(${PROJECT_NAME})

################################################################################################
################################################################################################

# Must be executed BEFORE add_executable!
macro(group_folder FOLDER)
	file(GLOB SOURCE_GROUP ${FOLDER}/*.c ${FOLDER}/*.h ${FOLDER}/*.cpp ${FOLDER}/*.hpp)
	string(REGEX REPLACE "/" "\\\\\\\\" SOURCE_GROUP_NAME ${FOLDER})
	source_group(${SOURCE_GROUP_NAME} FILES ${SOURCE_GROUP})
endmacro()

macro(add_test FILE_NAME)
	add_executable(${FILE_NAME} test/${FILE_NAME}.cpp test/test.h)
	target_link_libraries(${FILE_NAME} ${LUA_LIBRARIES})
	add_dependencies(ALL_TEST ${FILE_NAME})
endmacro()

################################################################################################
################################################################################################

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

################################################################################################
################################################################################################

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Tools/CMake")


group_folder("include")
set(INCLUDE_FILES ${SOURCE_GROUP})
include_directories(include)

if(LUA_INCLUDE_DIR)
	include_directories(${LUA_INCLUDE_DIR})
else(LUA_INCLUDE_DIR)
	find_package(Lua)
	include_directories(${LUA_INCLUDE_DIR})
endif(LUA_INCLUDE_DIR)

################################################################################################
################################################################################################

add_executable(ALL_TEST test/main.cpp ${INCLUDE_FILES})

add_test("state_test")
add_test("types_test")
add_test("get_test")
add_test("set_test")
add_test("ref_test")
add_test("lambda_test")
add_test("values_test")

################################################################################################
################################################################################################

# Flags for MacOS which links directly or indirectly against LuaJIT
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
