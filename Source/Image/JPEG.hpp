#ifndef __JPEG_HPP__
#define __JPEG_HPP__

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <cstdint>
#include <filesystem>

template <typename ImageT>
bool save_image(const ImageT &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
{
    constexpr auto channelCount = 4;

    return stbi_write_jpg(fullFilePath.c_str(), width, height, channelCount, image.data(), width * channelCount) != 0;
}

#endif // __JPEG_HPP__
