cmake_minimum_required(VERSION 2.8)
project(ROUTER_LAB LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)

set(BACKEND LINUX CACHE STRING "router platform")

add_compile_definitions(ROUTER_BACKEND_${BACKEND})
add_compile_definitions(ROUTER_NUMBER=${ROUTER_NUMBER})
if (USE_NETNS)
    add_compile_definitions(USE_NETNS)
endif()

add_subdirectory(HAL)
add_subdirectory(Homework/ripv2)
