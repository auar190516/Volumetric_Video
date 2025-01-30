#include <iostream>
#include <fstream>
#include "point_cloud_compressor.h"

bool fileExists(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();  // 如果文件打开成功，返回true
}
int main() {

    std::string input_file = "../build/Box.ply";  // 假设文件路径
    std::string compressed_file = "./test.drc";

    //检查文件是否存在
    if (!fileExists(input_file)) {
        std::cerr << "File " << input_file << " does not exist!" << std::endl;
        return 1;  // 如果文件不存在，返回错误码
    } else {
        std::cout << "File " << input_file << " exists!" << std::endl;
    }

    // 如果文件存在，执行压缩操作
    std::cout << "Starting compression..." << std::endl;
    compressPointCloud(input_file, compressed_file);

    return 0;
}
