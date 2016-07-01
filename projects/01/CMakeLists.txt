cmake_minimum_required(VERSION 3.5)
project(cs61c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    airports.txt
    flights.c
    flights.h
    routes.txt
    RouteTime.c
    schedules.txt
    timeHM.c
    timeHM.h)

add_executable(cs61c ${SOURCE_FILES})