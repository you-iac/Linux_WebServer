cmake_minimum_required(VERSION 3.28) #设置Cmake版本
project(main)                        #设置工程名称

include_directories (${PROJECT_SOURCE_DIR}inc)   #包含当前目录下头文件

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
aux_source_directory(src CPP_CODE)

add_executable(
        main
        ${CPP_CODE}
)        #提交生成的可执行文件main