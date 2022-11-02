# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("ALL")

#SET(CMAKE_BUILD_TYPE "Debug")

#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -static -O0 -Wall -g2 -ggdb")

#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -static -O3 -Wall")
# 包含子项目。
add_subdirectory ("free_lock")
add_subdirectory ("test")

