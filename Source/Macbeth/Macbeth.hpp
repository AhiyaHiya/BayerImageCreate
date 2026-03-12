#ifndef __MACBETH_HPP__
#define __MACBETH_HPP__

#include "Image/Bayer.hpp"
#include "Types/Types.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

constexpr auto RgbCount = 3U;

constexpr auto MacbethCols = 6U;
constexpr auto MacbethRows = 4U;

using rgb_colors_t = std::array<std::uint8_t, RgbCount>;

// These are RGB
constexpr auto MacbethPatches = std::array<rgb_colors_t, 24>{{
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

constexpr auto MacbethResolutions = std::array<std::pair<height_t, width_t>, 3>{{{612U, 792U},
                                                                                 {1275U, 1650U},
                                                                                 {2550U, 3300U}}};

template <typename ImageT>
struct MacbethImage
{
    const ImageT       RgbData;
    const width_t      PixelsWide;
    const height_t     PixelsHigh;
    const std::int32_t ChannelCount = 3U;
};

/*
 Uses Debayer layout to determine which color is returned
   for (0,0), (0,1), (1,0), (1,1)
*/
template <typename BitDepthT, typename C00 = BitDepthT, typename C01 = BitDepthT, typename C10 = BitDepthT, typename C11 = BitDepthT>
auto get_colors_for_bayer_layout(const DemosaicTypes bayerLayout, const rgb_colors_t &rgbColors)
    -> std::tuple<C00, C01, C10, C11>
{
    const auto &[r, g, b] = rgbColors;
    switch (bayerLayout)
    {
    case DemosaicTypes::COLOR_BayerBG2BGR:
        return {r, g, g, b}; // RGGB
    case DemosaicTypes::COLOR_BayerGB2BGR:
        return {g, r, b, g}; // GRBG
    case DemosaicTypes::COLOR_BayerRG2BGR:
        return {g, b, r, g}; // BGGR
    case DemosaicTypes::COLOR_BayerGR2BGR:
        return {b, g, g, r}; // GBRG
    default:
        throw std::runtime_error("Unsupported Bayer layout");
    }
}

/*
Returns interleaved image.
If Bayer format is RGGB:
Row 0 RGRGRGRGRGRGRGRG
Row 1 GBGBGBGBGBGBGBGB
*/
template <typename ImageT, typename BitDepthT = ImageT::value_type>
auto create_macbeth_colorchecker_image(const DemosaicTypes     bayerLayout,
                                       const MacbethResolution resolution = MacbethResolution::Screen) -> ImageT
{
    const auto &[pixelsHig2, pixelsWide] = MacbethResolutions[resolution];
    const int  blockSize                 = pixelsWide / MacbethCols;
    const auto pixelsHigh                = blockSize * MacbethRows;
    const auto expectedPixelCount        = pixelsWide * pixelsHigh * BayerChannelCount;
    auto       image                     = ImageT(pixelsWide * pixelsHigh * BayerChannelCount, 0);

    auto           colorIndex = 0;
    constexpr auto gridCount  = 2U;
    for (auto r = 0; r < MacbethRows; ++r)
    {
        const auto rowStart = r * blockSize;
        const auto rowStop  = rowStart + blockSize;

        for (auto c = 0; c < MacbethCols; ++c)
        {
            const auto colStart = c * blockSize;
            const auto colStop  = colStart + blockSize;

            const auto &rgbColors            = MacbethPatches[colorIndex];
            const auto &[c00, c01, c10, c11] = get_colors_for_bayer_layout<BitDepthT>(bayerLayout, rgbColors);

            for (auto y = rowStart; y < rowStop; ++y)
            {
                for (auto x = colStart; x < colStop; ++x)
                {
                    const auto offset0 = (y * (pixelsWide * gridCount)) + (x * gridCount);
                    const auto offset1 = (y + 1 * (pixelsWide * gridCount)) + (x * gridCount);

                    image[offset0 + 0] = c00;
                    image[offset0 + 1] = c01;
                    image[offset1 + 0] = c10;
                    image[offset1 + 1] = c11;
                }
            }
            ++colorIndex;
        }
    }
    return image;
}

template <typename ImageT>
auto CreateMacbethImage(const MacbethResolution resolution) -> MacbethImage<ImageT>
{
    const auto &[pixelsHigh, pixelsWide] = MacbethResolutions[resolution];
    auto rgbData                         = create_macbeth_colorchecker_data<ImageT>(resolution);
    auto macbethImage                    = MacbethImage{
                           .RgbData    = std::move(rgbData),
                           .PixelsWide = pixelsWide,
                           .PixelsHigh = pixelsHigh};
    return macbethImage;
}

/*
 Output is standard Macbeth ColorChecker image
 Data is Interleaved RGB, e.g. RGBRGBRGBRGB
 Bit depth is set by ImageT::value_type, so either uint8_t or uint16_t
 Resolution is set by MacbethResolution
    Output is RGB data, that is for a .
 */
template <typename ImageT, typename BitDepthT = ImageT::value_type>
auto create_macbeth_colorchecker_data(const MacbethResolution resolution) -> ImageT
{
    const auto &[pixelsHigh, pixelsWide] = MacbethResolutions[resolution];
    const int blockSize                  = pixelsWide / MacbethCols;

    // Create a container big enough to hold an image that is pixelsWide * pixelsHigh in total pixels
    // and contains 3 samples per pixel
    auto image = ImageT(pixelsWide * pixelsHigh * RgbCount, 0);

    auto colorIndex = 0;
    for (auto r = 0; r < MacbethRows; ++r)
    {
        const auto rowStart = r * blockSize;
        const auto rowStop  = rowStart + blockSize;

        for (auto c = 0; c < MacbethCols; ++c)
        {
            const auto colStart = c * blockSize;
            const auto colStop  = colStart + blockSize;

            const auto &[r, g, b] = MacbethPatches[colorIndex];

            for (auto y = rowStart; y < rowStop; ++y)
            {
                for (auto x = colStart; x < colStop; ++x)
                {
                    const auto offset = (y * (pixelsWide * RgbCount)) + (x * RgbCount);

                    image[offset + 0] = r;
                    image[offset + 1] = g;
                    image[offset + 2] = b;
                }
            }
            ++colorIndex;
        }
    }
    return image;
}

#endif // __MACBETH_HPP__
