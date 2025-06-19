cmake_minimum_required(VERSION 3.10)
project(main)          # 修改为你的项目名

set(CMAKE_CXX_STANDARD 17)          # 设置C++标准

# 添加可执行文件（需替换实际源文件）
add_executable( ${PROJECT_NAME}
        main.cpp
        15线程池/15-1processpool.h
)