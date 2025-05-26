cmake_minimum_required(VERSION 3.31)
project(http)

set(CMAKE_CXX_STANDARD 20)

add_executable(http

#        listen.cpp
        sockread.cpp
        8_3.cpp
        9_4.cpp
        9_3.cpp
        9_5.cpp
        "9_6 ssh_client.cpp"
        "9_7 ssh_server.cpp"
        #        sockwrite.cpp
)
