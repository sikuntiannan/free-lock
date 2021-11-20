# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("test")

include_directories(../free_lock)

link_directories(../out/install/lib)

add_executable (test main.cpp)

target_link_libraries(test free_lock)

INSTALL (TARGETS test  test DESTINATION bin)