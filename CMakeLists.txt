cmake_minimum_required (VERSION 2.8.9)

#SET(CMAKE_C_FLAGS "-pg")
SET(CMAKE_BUILD_TYPE "DEBUG")

add_executable(pqcomp src/pqcomp.c src/pqlist.c src/pqheap.c)

