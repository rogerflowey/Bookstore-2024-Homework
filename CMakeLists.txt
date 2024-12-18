cmake_minimum_required(VERSION 3.28)

project(BookStore)


set(CMAKE_CXX_STANDARD 20)

# 将源文件添加到项目中
add_executable(main main.cpp
        myError.h)