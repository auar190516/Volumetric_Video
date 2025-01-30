cmake_minimum_required(VERSION 3.10)
project(PointCloudServer)

# 设置C++版本
set(CMAKE_CXX_STANDARD 14)

# 手动指定Draco的安装路径
# 如果Draco没有在标准路径下安装，请根据你的实际安装路径修改以下路径
set(DRACO_INCLUDE_DIR "/usr/local/include/draco")
find_library(DRACO_LIB NAMES draco PATHS /usr/local/lib)

# 如果找不到Draco库，抛出错误
if (NOT DRACO_LIB)
    message(FATAL_ERROR "Draco library not found!")
endif()



# 设置源文件
set(SOURCES
    src/main.cpp
    src/point_cloud_compressor.cpp
)

# 创建可执行文件
add_executable(PointCloudServer ${SOURCES})

# 添加Draco头文件目录
target_include_directories(PointCloudServer PRIVATE ${DRACO_INCLUDE_DIR})

# 链接Draco库
target_link_libraries(PointCloudServer 
    -Wl,--whole-archive 
    ${DRACO_LIB} 
    -Wl,--no-whole-archive)
