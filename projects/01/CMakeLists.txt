cmake_minimum_required(VERSION 3.5)
project(01)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    airports.txt
    flights.c
    flights.h
    flights.out
    Makefile
    routes.txt
    RouteTime.c
    schedules.txt
    timeHM.c
    timeHM.h
    timeHM.o)

add_executable(01 ${SOURCE_FILES})