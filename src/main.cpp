#include <iostream>
#include <fstream>
#include "point_cloud_compressor.h"
#include <boost/asio.hpp>
#include <thread>

void sendCompressedData(boost::asio::ip::tcp::socket &socket, const std::vector<char> &data) {
    try {
        int data_size = data.size();

        std::cout << "compressed data length: " << data_size << std::endl;
        boost::asio::write(socket, boost::asio::buffer(&data_size, sizeof(data_size)));
        std::size_t bytes_sent = boost::asio::write(socket, boost::asio::buffer(data));
        if (bytes_sent == data.size()) {
            std::cout << "Sent compressed data to client." << std::endl;
        } else {
            std::cerr << "Failed to send full data: sent " << bytes_sent << " bytes, expected " << data.size() << " bytes." << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error sending data: " << e.what() << std::endl;
    }
}

void handleClient(boost::asio::ip::tcp::socket socket, const std::vector<char> &compressed_data) {
    while (true) {
        sendCompressedData(socket, compressed_data);
        // 休眠5秒
        std::this_thread::sleep_for(std::chrono::seconds(15));
    }
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
    std::string input_file = "../build/longdress_vox10_1148.ply";  // 假设文件路径
    int port = 12348;  // 服务器监听的端口

    startServer(input_file, port);

    return 0;
}