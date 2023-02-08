cmake_minimum_required(VERSION 3.22)
project(FTPClient C)

set(CMAKE_C_STANDARD 99)

add_executable(FTPClient main.c handler.c handler.h connect.c connect.h ftp.c ftp.h)
