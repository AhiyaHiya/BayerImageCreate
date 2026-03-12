#ifndef __JPEG_HPP__
#define __JPEG_HPP__

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <cstdint>
#include <filesystem>

template <typename ImageT>
bool save_image(const ImageT &macbethImage, const std::filesystem::path fullFilePath)
{
    return stbi_write_jpg(fullFilePath.c_str(), macbethImage.PixelsWide, macbethImage.PixelsHigh, macbethImage.ChannelCount,
                          macbethImage.RgbData.data(), macbethImage.PixelsWide * macbethImage.ChannelCount) != 0;
}

#endif // __JPEG_HPP__
