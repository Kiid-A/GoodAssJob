cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 设置 C++ 标准
set( CMAKE_CXX_FLAGS "-std=c++11" )

# 添加一个可执行文件
add_executable(Myproject main.cpp Article.cpp DatabaseManager.cpp sqlite3.c)
