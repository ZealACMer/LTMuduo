cmake_minimum_required(VERSION 2.5)
project(ltmuduo)

#设置调试信息以及启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

#ltmuduo最终编译成so动态库，设置动态库的路径，放在根目录的lib文件夹下面
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I
include_directories(${PROJECT_SOURCE_DIR}/include)

# 在src下继续寻找cmakelists生成makefile，用于整个项目的编译
add_subdirectory(src)