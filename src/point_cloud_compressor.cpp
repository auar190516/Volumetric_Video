#include <cinttypes>
#include <cstdlib>

#include "draco/compression/config/compression_shared.h"
#include "draco/compression/encode.h"
#include "draco/compression/expert_encode.h"
#include "draco/core/cycle_timer.h"
#include "draco/io/file_utils.h"
#include "draco/io/mesh_io.h"
#include "draco/io/point_cloud_io.h"
#include <string>
#include <memory>

void compressPointCloud(const std::string &input_file, const std::string &output_file) {
    // Step 1: Load the point cloud from the input PLY file.
    auto maybe_pc = draco::ReadPointCloudFromFile(input_file);
    if (!maybe_pc.ok()) {
        printf("Failed loading the input point cloud: %s.\n", maybe_pc.status().error_msg());
        return;
    }
    std::unique_ptr<draco::PointCloud> point_cloud = std::move(maybe_pc).value();

    // Step 2: Set up the encoder with quantization and compression settings.
    draco::ExpertEncoder encoder(*point_cloud);  // Use ExpertEncoder for EncodeToBuffer functionality.

    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);  // Default position quantization to 11 bits.
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);  // Default texture quantization to 10 bits.
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 8);     // Default normal quantization to 8 bits.

    // Set the compression level (0-10, 10 being the fastest).
    int compression_speed = 7;  // Default compression level.
    encoder.SetSpeedOptions(10 - compression_speed, 10 - compression_speed); 

    // Step 3: Encode the point cloud to buffer.
    draco::CycleTimer timer;
    draco::EncoderBuffer buffer;
    timer.Start();
    const draco::Status status = encoder.EncodeToBuffer(&buffer);
    if (!status.ok()) {
        printf("Failed to encode the point cloud: %s\n", status.error_msg());
        return;
    }
    timer.Stop();

    // Step 4: Write the encoded point cloud data to the output file.
    if (!draco::WriteBufferToFile(buffer.data(), buffer.size(), output_file)) {
        printf("Failed to write the encoded point cloud to the output file: %s\n", output_file.c_str());
        return;
    }

    printf("Point cloud successfully encoded and saved to %s (%" PRId64 " ms to encode).\n", output_file.c_str(), timer.GetInMs());
    printf("Encoded size = %zu bytes\n\n", buffer.size());
}
