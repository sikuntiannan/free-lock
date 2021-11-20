# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("free_lock")

add_definitions(-DDLL_EXPORTING)

set( CMAKE_CXX_STANDARD 17)

include_directories(./)

set(lib_src "lock.cpp")

add_library(free_lock STATIC ${lib_src} )

INSTALL (TARGETS free_lock  free_lock DESTINATION lib)

INSTALL (FILES lock.h DESTINATION include/free-lock/)
INSTALL (FILES queue.h DESTINATION include/free-lock/)
INSTALL (FILES Head.h DESTINATION include/free-lock/)
INSTALL (FILES ErrorDef.h DESTINATION include/free-lock/)