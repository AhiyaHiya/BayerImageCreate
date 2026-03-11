#include <cstdint>
#include <filesystem>
#include <iostream>
// #include <mdspan> // LLVM 21 doesn't not have mdspan and neither does GCC 15
#include <vector>

// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

namespace
{
constexpr auto channelCount = 4U; // RGGB

using colors_t = std::array<std::uint8_t, channelCount>;

// These are RGB
constexpr auto macbethPatches = std::array<colors_t, 24>{{
    {{115, 82, 68}},   // 01 Dark skin
    {{194, 150, 130}}, // 02 Light skin
    {{98, 122, 157}},  // 03 Blue sky
    {{87, 108, 67}},   // 04 Foliage
    {{133, 128, 177}}, // 05 Blue flower
    {{103, 189, 170}}, // 06 Bluish green
    {{214, 126, 44}},  // 07 Orange
    {{80, 91, 166}},   // 08 Purplish blue
    {{193, 90, 99}},   // 09 Moderate red
    {{94, 60, 108}},   // 10 Purple
    {{157, 188, 64}},  // 11 Yellow green
    {{224, 163, 46}},  // 12 Orange yellow
    {{56, 61, 150}},   // 13 Blue
    {{70, 148, 73}},   // 14 Green
    {{175, 54, 60}},   // 15 Red
    {{231, 199, 31}},  // 16 Yellow
    {{187, 86, 149}},  // 17 Magenta
    {{8, 133, 161}},   // 18 Cyan     (note: this is out-of-gamut in strict sRGB; clipped somewhat)
    {{243, 243, 242}}, // 19 White     (slightly off-white in real charts)
    {{200, 200, 200}}, // 20 Neutral 8
    {{160, 160, 160}}, // 21 Neutral 6.5
    {{122, 122, 121}}, // 22 Neutral 5
    {{85, 85, 85}},    // 23 Neutral 3.5
    {{52, 52, 52}}     // 24 Black
}};

enum MacbethResolution : std::uint8_t
{
    Screen = 0,
    LowResPrint,
    HighQualityPrint
};

constexpr auto macbethResolutions = std::array<std::pair<std::uint32_t, std::uint32_t>, 3>{{{612U, 792U}, {1275U, 1650U}, {2550U, 3300U}}};

// These values copied from modules/imgproc/include/opencv2/imgproc.hpp
enum class DemosaicTypes : std::int32_t
{
    COLOR_BayerBG2BGR = 46, //!< [8U/16U] equivalent to RGGB Bayer pattern
    COLOR_BayerGB2BGR = 47, //!< [8U/16U] equivalent to GRBG Bayer pattern
    COLOR_BayerRG2BGR = 48, //!< [8U/16U] equivalent to BGGR Bayer pattern
    COLOR_BayerGR2BGR = 49, //!< [8U/16U] equivalent to GBRG Bayer pattern

    COLOR_BayerRGGB2BGR = COLOR_BayerBG2BGR, //!< [8U/16U]
    COLOR_BayerGRBG2BGR = COLOR_BayerGB2BGR, //!< [8U/16U]
    COLOR_BayerBGGR2BGR = COLOR_BayerRG2BGR, //!< [8U/16U]
    COLOR_BayerGBRG2BGR = COLOR_BayerGR2BGR, //!< [8U/16U]

    COLOR_BayerRGGB2RGB = COLOR_BayerBGGR2BGR, //!< [8U/16U]
    COLOR_BayerGRBG2RGB = COLOR_BayerGBRG2BGR, //!< [8U/16U]
    COLOR_BayerBGGR2RGB = COLOR_BayerRGGB2BGR, //!< [8U/16U]
    COLOR_BayerGBRG2RGB = COLOR_BayerGRBG2BGR, //!< [8U/16U]

    COLOR_BayerBG2RGB = COLOR_BayerRG2BGR, //!< [8U/16U] equivalent to RGGB Bayer pattern
    COLOR_BayerGB2RGB = COLOR_BayerGR2BGR, //!< [8U/16U] equivalent to GRBG Bayer pattern
    COLOR_BayerRG2RGB = COLOR_BayerBG2BGR, //!< [8U/16U] equivalent to BGGR Bayer pattern
    COLOR_BayerGR2RGB = COLOR_BayerGB2BGR, //!< [8U/16U] equivalent to GBRG Bayer pattern
};

using Image1D_8U  = std::vector<std::uint8_t>;
using Image1D_16U = std::vector<std::uint16_t>;

// Returns the indexes of the RGGB channels in the order of R, G0, G1, B for the given Bayer layout
auto get_rggb_indexes(const DemosaicTypes bayerLayout) -> std::tuple<int32_t, int32_t, int32_t, int32_t>
{
    switch (bayerLayout)
    {
    case DemosaicTypes::COLOR_BayerBG2BGR:
        return {0, 1, 2, 3}; // RGGB
    case DemosaicTypes::COLOR_BayerGB2BGR:
        return {1, 0, 3, 2}; // GRBG
    case DemosaicTypes::COLOR_BayerRG2BGR:
        return {2, 3, 0, 1}; // BGGR
    case DemosaicTypes::COLOR_BayerGR2BGR:
        return {3, 2, 1, 0}; // GBRG
    default:
        throw std::runtime_error("Unsupported Bayer layout");
    }
}

template <typename ImageT>
auto create_macbeth_colorchecker_image(const DemosaicTypes     bayerLayout,
                                       const MacbethResolution resolution = MacbethResolution::Screen) -> ImageT
{
    const auto    &res        = macbethResolutions[resolution];
    const auto     pixelsWide = res.first;
    const auto     pixelsHigh = res.second;
    constexpr auto cols       = 6U;
    constexpr auto rows       = 4U;

    const int blockSize = pixelsWide / cols;

    auto image = ImageT(pixelsWide * pixelsHigh * channelCount, 0);
    // auto image2D = std::mdspan(image.data(), pixelsHigh * 2, pixelsWide * 2);

    const auto [rIndex, g0Index, g1Index, bIndex] = get_rggb_indexes(bayerLayout);
    auto colorIndex                               = 0;
    for (auto r = 0; r < rows; ++r)
    {
        for (auto c = 0; c < cols; ++c)
        {
            for (auto y = r * blockSize; y < r * blockSize + blockSize; ++y)
            {
                for (auto x = c * blockSize; x < c * blockSize + blockSize; ++x)
                {
                    const auto &color = macbethPatches[colorIndex];
                    const auto  r     = color[rIndex];
                    const auto  g0    = color[g0Index];
                    const auto  g1    = color[g1Index];
                    const auto  b     = color[bIndex];

                    // const auto x0            = x * channelCount;
                    const auto offset = (y * width + x) * channelCount;
                    // image2D[y][x0 + rIndex]  = r;
                    // image2D[y][x0 + g0Index] = g0;
                    // image2D[y][x0 + g1Index] = g1;
                    // image2D[y][x0 + bIndex]  = b;
                    image[offset + rIndex]  = r;
                    image[offset + g0Index] = g0;
                    image[offset + g1Index] = g1;
                    image[offset + bIndex]  = b;
                }
            }
            ++colorIndex;
        }
    }
    return image;
}

// template <typename ImageT>
// bool save_image(const ImageT &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
// {
//     constexpr auto channelCount = 4;

//     return stbi_write_tiff(fullFilePath.c_str(), width, height, channelCount, image.data(), width * channelCount) != 0;
// }

template <typename ImageT>
bool save_image_as_tiff(const ImageT &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
{
    return false;
}

}; // namespace

int main(int, char **)
{
    std::cout << "Hello!\n";

    constexpr auto pixelsWide = 792U;
    constexpr auto pixelsHigh = 612U;

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR);
    (void)image;
    // const auto result = save_image(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff", pixelsWide, pixelsHigh);
    // std::cout << "Image save result: " << (result ? "Success\n" : "Fail\n");

    return 0;
}
