cmake_minimum_required(VERSION 3.5)
project(04)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    test/expected/cal1-expected.txt
    test/expected/cal2-expected.txt
    test/expected/empty-expected.txt
    test/expected/quilt1-expected.txt
    test/expected/quilt2-expected.txt
    test/expected/quilt3-expected.txt
    test/expected/quilt4-expected.txt
    test/expected/real1-expected.txt
    test/expected/real2-expected.txt
    test/expected/small-expected.txt
    test/expected/tiny-expected.txt
    test/images/cal1-left.bmp
    test/images/cal1-right.bmp
    test/images/cal2-left.bmp
    test/images/cal2-right.bmp
    test/images/empty-input.bmp
    test/images/quilt1-left.bmp
    test/images/quilt1-right.bmp
    test/images/quilt2-left.bmp
    test/images/quilt2-right.bmp
    test/images/quilt3-left.bmp
    test/images/quilt3-right.bmp
    test/images/quilt4-left.bmp
    test/images/quilt4-right.bmp
    test/images/real1-left.bmp
    test/images/real1-right.bmp
    test/images/real2-left.bmp
    test/images/real2-right.bmp
    test/images/small-input.bmp
    test/images/tiny-input.bmp
    test/output/cal1-output.bmp
    test/output/cal1-output.txt
    test/output/cal2-output.bmp
    test/output/cal2-output.txt
    test/output/empty-output.txt
    test/output/quilt1-output.bmp
    test/output/quilt1-output.txt
    test/output/quilt2-output.bmp
    test/output/quilt2-output.txt
    test/output/quilt3-output.bmp
    test/output/quilt3-output.txt
    test/output/quilt4-output.bmp
    test/output/quilt4-output.txt
    test/output/real1-output.bmp
    test/output/real1-output.txt
    test/output/real2-output.bmp
    test/output/real2-output.txt
    test/output/small-output.txt
    test/output/tiny-output.txt
    benchmark.c
    calcDepthNaive.c
    calcDepthNaive.h
    calcDepthOptimized.c
    calcDepthOptimized.h
    check.c
    depthMap.c
    Makefile
    utils.c
    utils.h)

add_executable(04 ${SOURCE_FILES})