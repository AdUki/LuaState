cmake_minimum_required(VERSION 2.8)

set (ProjectName "LuaState")

project(${ProjectName})

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")
set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

# Must be executed BEFORE add_executable!
macro(add_folder FOLDER)
	file(GLOB SOURCE_GROUP ${FOLDER}/*.c ${FOLDER}/*.h ${FOLDER}/*.cpp ${FOLDER}/*.hpp)
	string(REGEX REPLACE "/" "\\\\\\\\" SOURCE_GROUP_NAME ${FOLDER})
	source_group(${SOURCE_GROUP_NAME} FILES ${SOURCE_GROUP})
	set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_GROUP})
endmacro()

add_folder("include")
add_folder("test")

include_directories(include)

find_package(LuaJIT)
include_directories(${LUAJIT_INCLUDE_DIR})

add_executable(${ProjectName} ${SOURCE_FILES})
target_link_libraries(${ProjectName} ${LUAJIT_LIBRARY})

# Flags for MacOS which links directly or indirectly against LuaJIT
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
