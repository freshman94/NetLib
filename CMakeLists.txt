cmake_minimum_required (VERSION 2.8)

project(NetLib)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

message(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXXFLAGS: " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(Boost REQUIRED)

set(SRC_LIST "log" "base" "net" "http")

foreach(SUB_DIR ${SRC_LIST})
	aux_source_directory(src/${SUB_DIR} SUB_LIST)
	add_library(NetLib_${SUB_DIR} ${SUB_LIST})
	target_link_libraries(NetLib_${SUB_DIR} pthread)
	install(TARGETS NetLib_${SUB_DIR} DESTINATION lib)
	install(DIRECTORY src/${SUB_DIR}/ DESTINATION include FILES_MATCHING PATTERN "*.h")
endforeach()


add_subdirectory(src/base/tests)
add_subdirectory(src/log/tests)
add_subdirectory(src/net/tests)
add_subdirectory(src/http)

add_subdirectory(bench/pingpong)
	