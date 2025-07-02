# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(ethercat_control)

# set(CMAKE_C_STANDARD 11)

<<<<<<< HEAD
# set(compile_commands_dir ${CMAKE_BINARY_DIR}/compile_commands) # for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 假设IGH安装在默认路径，根据实际情况修改
=======
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

>>>>>>> a4d5802 (second commit)
include_directories(/home/gbb/ec_igh/ethercat/include
                   )

link_directories(/home/gbb/ec_igh/ethercat/lib/.libs)

add_executable(ethercat_control
    src/main.c
)

target_link_libraries(ethercat_control ethercat pthread rt)
