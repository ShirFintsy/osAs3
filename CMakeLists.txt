cmake_minimum_required(VERSION 3.22)
project(ex3)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)

add_executable(ex3 main.cpp BoundedQueue.cpp BoundedQueue.h semaphore.cpp semaphore.h UnboundedQueue.cpp UnboundedQueue.h)
