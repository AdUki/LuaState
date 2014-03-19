cmake_minimum_required(VERSION 2.8.12)

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

set (PROJECT "LuaState")
project(${PROJECT})

file(GLOB SOURCE_GROUP "include/*.cpp" "include/*.h")
source_group("include" FILES ${SOURCE_GROUP})
set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_GROUP})

file(GLOB SOURCE_GROUP "test/*.cpp" "test/*.h")
source_group("test" FILES ${SOURCE_GROUP})
set(SOURCE_FILES ${SOURCE_FILES} ${SOURCE_GROUP})

# Create executable
add_executable(${PROJECT} ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

find_package(LuaJIT)
if(LUAJIT_FOUND)

	include_directories(${LUAJIT_INCLUDE_DIR})
	target_link_libraries(${PROJECT} ${LUAJIT_LIBRARY})

	# Flags for MacOS which links directly or indirectly against LuaJIT
	if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pagezero_size 10000 -image_base 100000000")
	endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

else(LUAJIT_FOUND)

	message( FATAL_ERROR "LuaJIT not found!" )

endif(LUAJIT_FOUND)
