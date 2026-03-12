#include "Image/Bayer.hpp"
#include "Image/Tiff.hpp"
#include "Macbeth/Macbeth.hpp"
#include "Types/Types.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
// #include <mdspan> // LLVM 21 doesn't not have mdspan and neither does GCC 15
#include <vector>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

namespace
{

// template <typename ImageT>
// bool save_image(const ImageT &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
// {
//     constexpr auto channelCount = 4;

//     return stbi_write_tiff(fullFilePath.c_str(), width, height, channelCount, image.data(), width * channelCount) != 0;
// }

}; // namespace

int main(int, char **)
{
    std::cout << "Hello!\n";

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR, MacbethResolution::Screen);

    const auto [height2, width] = MacbethResolutions[MacbethResolution::Screen];
    const auto height           = (width / 6U) * 4U;
    const auto result           = save_image_as_tiff<Image1D_8U>(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff",
                                                                 width, height);
    // const auto result = save_image(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff", pixelsWide, pixelsHigh);
    // std::cout << "Image save result: " << (result ? "Success\n" : "Fail\n");

    return result == true ? 0 : 1;
}
