cmake_minimum_required (VERSION 2.6)
project (panwave_test)

include_directories (${PROJECT_SOURCE_DIR}/src)

set (SOURCES ${PROJECT_SOURCE_DIR}/src/Wavelet.cc
         ${PROJECT_SOURCE_DIR}/src/WaveletMath.cc
         ${PROJECT_SOURCE_DIR}/src/WaveletPacketTree.cc
         ${PROJECT_SOURCE_DIR}/src/StationaryWaveletPacketTree.cc
         ${PROJECT_SOURCE_DIR}/test/test.cc)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++17 -Wall")

add_executable (panwave_test ${SOURCES})
