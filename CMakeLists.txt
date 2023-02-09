cmake_minimum_required (VERSION 3.7.2 FATAL_ERROR)

find_program (CLANG_TIDY_EXE NAMES "clang-tidy")
if (CLANG_TIDY_EXE)
  set (CMAKE_CXX_CLANG_TIDY clang-tidy)
endif ()

project (panwave_test)

include_directories (${PROJECT_SOURCE_DIR}/src)

set (SOURCES ${PROJECT_SOURCE_DIR}/src/Wavelet.cc
         ${PROJECT_SOURCE_DIR}/src/WaveletMath.cc
         ${PROJECT_SOURCE_DIR}/src/WaveletPacketTree.cc
         ${PROJECT_SOURCE_DIR}/src/StationaryWaveletPacketTree.cc
         ${PROJECT_SOURCE_DIR}/test/test.cc)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable (panwave_test ${SOURCES})

if (MSVC)
  # disable some benign warnings on MSVC
  target_compile_options (panwave_test PRIVATE "/Wall;/wd4514;/wd4625;/wd4626;/wd5026;/wd5027;/wd5045;/wd4710;/wd4820;")
else ()
  target_compile_options (panwave_test PRIVATE "-Wall;-g;-O3;")
endif ()
