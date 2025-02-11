#include <iostream>
#include <fstream>
#include "point_cloud_compressor.h"
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <condition_variable>

std::vector<std::thread> client_threads;
std::vector<char> current_compressed_data;
std::mutex data_mutex;
std::condition_variable data_cv;
bool new_data_available = false;
bool server_running = true;

void sendCompressedData(boost::asio::ip::tcp::socket &socket) {
    try {
        while (server_running) {
            std::unique_lock<std::mutex> lock(data_mutex);
            data_cv.wait(lock, [] { return new_data_available; });

            int data_size = current_compressed_data.size();
            std::cout << "compressed data length: " << data_size << std::endl;
            boost::asio::write(socket, boost::asio::buffer(&data_size, sizeof(data_size)));
            std::size_t bytes_sent = boost::asio::write(socket, boost::asio::buffer(current_compressed_data));
            if (bytes_sent == current_compressed_data.size()) {
                std::cout << "Sent compressed data to client." << std::endl;
            } else {
                std::cerr << "Failed to send full data: sent " << bytes_sent << " bytes, expected " << current_compressed_data.size() << " bytes." << std::endl;
            }

            new_data_available = false;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error sending data: " << e.what() << std::endl;
    }
}

void handleClient(boost::asio::ip::tcp::socket socket) {
    sendCompressedData(socket);
}

void fileLoader(const std::string &base_filename, int start_index) {
    int index = start_index;
    while (server_running) {
        std::string input_file = base_filename + std::to_string(index) + ".ply";
        std::vector<char> compressed_data;
        compressPointCloud(input_file, compressed_data);

        {
            std::lock_guard<std::mutex> lock(data_mutex);
            current_compressed_data = compressed_data;
            new_data_available = true;
        }
        data_cv.notify_all();

        index++;
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
}

void startServer(const std::string &base_filename, int start_index, int port) {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::acceptor acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port));

    std::cout << "Server started, waiting for connections on port " << port << "..." << std::endl;

    // 启动文件加载线程
    std::thread loader_thread(fileLoader, base_filename, start_index);

    while (true) {
        // 等待客户端连接
        boost::asio::ip::tcp::socket socket(io_service);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // 创建新线程处理客户端请求
        client_threads.emplace_back(handleClient, std::move(socket));
    }

    // 等待文件加载线程结束
    loader_thread.join();
    for (auto &thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

int main() {
    std::string base_filename = "../../longdress/longdress/Ply/longdress_vox10_";  // 基础文件名
    int start_index = 1148;  // 起始索引
    int port = 12349;  // 服务器监听的端口

    startServer(base_filename, start_index, port);

    server_running = false;
    data_cv.notify_all();

    return 0;
}