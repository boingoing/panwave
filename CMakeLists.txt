cmake_minimum_required (VERSION 3.7.2 FATAL_ERROR)

find_program (CLANG_TIDY_EXE NAMES "clang-tidy")
if (CLANG_TIDY_EXE)
  set (CMAKE_CXX_CLANG_TIDY clang-tidy)
endif ()

project (panwave)

include_directories (${PROJECT_SOURCE_DIR}/src)

set (LIB_SOURCES ${PROJECT_SOURCE_DIR}/src/Wavelet.cc
  ${PROJECT_SOURCE_DIR}/src/WaveletMath.cc
  ${PROJECT_SOURCE_DIR}/src/WaveletPacketTree.cc
  ${PROJECT_SOURCE_DIR}/src/StationaryWaveletPacketTree.cc)
add_library (panwave STATIC ${LIB_SOURCES})

set (TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/test.cc)
add_executable (panwave_test ${TEST_SOURCES})
target_link_libraries (panwave_test panwave)

set (CMAKE_CXX_STANDARD 17)
if (MSVC)
  # disable some benign warnings on MSVC
  add_compile_options ("/Wall;/wd4514;/wd4625;/wd4626;/wd5026;/wd5027;/wd5045;/wd4710;/wd4820;")
else ()
  add_compile_options ("-Wall;")
endif ()
