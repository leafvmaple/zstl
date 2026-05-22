cmake_minimum_required(VERSION 3.16)
project(zstl LANGUAGES CXX)

add_library(zstl INTERFACE)
add_library(zstl::zstl ALIAS zstl)

target_include_directories(zstl INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(zstl INTERFACE cxx_std_17)
