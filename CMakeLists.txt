cmake_minimum_required(VERSION 2.8)

# Passing in values: http://stackoverflow.com/questions/12896988/passing-the-argument-to-cmake-via-command-prompt
# Actually this looks better http://stackoverflow.com/questions/5998186/cmake-adding-command-line-options

SET(PROJECT "litmus_driver")

# In case you want to give a special prefix to the project
SET(EXE_NAME "litmus_exe")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

project(${PROJECT} CXX)

# Set module path to find OpenCL
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}) 

# Find and include opencl
find_package(OpenCL REQUIRED)
include_directories(${OPENCL_INCLUDE_DIR})


#if windows, get the VS getopt.h file
#if (WIN32)
#include_directories(${GGC_BASE}/rt-opencl/ext/VS_getopt)
#endif()

# Link the runtime library
# LINK_DIRECTORIES(${GGC_BASE}/rt-opencl/lib) 

add_executable(${EXE_NAME}
 litmus_driver.cpp
)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
target_link_libraries(${EXE_NAME} ${OPENCL_LIBRARIES})

