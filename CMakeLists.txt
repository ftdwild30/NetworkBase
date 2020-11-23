cmake_minimum_required(VERSION 2.8)

project(NETWORK_BASE)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_BINARY_DIR})

# epoll/select model
include(CheckIncludeFile)
CHECK_INCLUDE_FILE(sys/epoll.h SUPPORT_EPOLL)
if(SUPPORT_EPOLL)
    message(STATUS "Support epoll")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIO_MULTIPLEXING_EPOLL")
else()
    message(STATUS "Support select")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIO_MULTIPLEXING_SELECT")
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)