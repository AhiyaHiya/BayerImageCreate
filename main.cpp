#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using Image2D = std::vector<std::vector<std::uint8_t>>;
bool save_image(const Image2D &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height);

int main(int, char **) { std::cout << "Hello!\n"; }

bool save_image(const Image2D &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
{
    constexpr auto channelCount = 3;

    auto flat = std::vector<std::uint8_t>();
    flat.reserve(width * height * channelCount);
    for (const auto &row : image)
    {
        flat.insert(flat.end(), row.begin(), row.end());
    }
    return stbi_write_tiff(fullFilePath.c_str(), width, height, channelCount, flat.data(), width * channelCount) != 0;
}
