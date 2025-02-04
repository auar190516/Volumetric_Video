#include <iostream>
#include <fstream>
#include "point_cloud_compressor.h"
#include <boost/asio.hpp>
#include <thread>

void sendCompressedData(boost::asio::ip::tcp::socket &socket, const std::vector<char> &data) {
    try {
        boost::asio::write(socket, boost::asio::buffer(data));
        std::cout << "Sent compressed data to client." << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error sending data: " << e.what() << std::endl;
    }
}

void handleClient(boost::asio::ip::tcp::socket socket, const std::vector<char> &compressed_data) {
    sendCompressedData(socket, compressed_data);
}

void startServer(const std::string &input_file, int port) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    std::cout << "Server started, waiting for connections on port " << port << "..." << std::endl;

    // 加载并压缩点云数据
    std::vector<char> compressed_data;
    compressPointCloud(input_file, compressed_data);

    while (true) {
        // 等待客户端连接
        boost::asio::ip::tcp::socket socket(io_service);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // 创建新线程处理客户端请求
        std::thread(&handleClient, std::move(socket), std::cref(compressed_data)).detach();
    }
}

int main() {
    std::string input_file = "../build/Box.ply";  // 假设文件路径
    int port = 12345;  // 服务器监听的端口

    startServer(input_file, port);

    return 0;
}

