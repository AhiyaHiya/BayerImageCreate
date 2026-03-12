#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
// #include <mdspan> // LLVM 21 doesn't not have mdspan and neither does GCC 15
#include <vector>

#include <tiffio.h>
// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include "stb_image_write.h"

namespace
{
constexpr auto channelCount = 4U; // RGGB
constexpr auto rgbCount     = 3U;

using rgb_colors_t = std::array<std::uint8_t, rgbCount>;

// These are RGB
constexpr auto macbethPatches = std::array<rgb_colors_t, 24>{{
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

using height_t = std::uint32_t;
using width_t  = std::uint32_t;

constexpr auto macbethResolutions = std::array<std::pair<height_t, width_t>, 3>{{{612U, 792U},
                                                                                 {1275U, 1650U},
                                                                                 {2550U, 3300U}}};

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

/*
 Uses Debayer layout to determine which color is returned
   for (0,0), (0,1), (1,0), (1,1)
*/
template <typename BitDepthT, typename C00 = BitDepthT, typename C01 = BitDepthT, typename C10 = BitDepthT, typename C11 = BitDepthT>
auto get_colors_for_bayer_layout(const DemosaicTypes bayerLayout, const rgb_colors_t &rgbColors, const std::uint16_t colorIndex)
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
    constexpr auto cols = 6U;
    constexpr auto rows = 4U;

    const auto &[pixelsHig2, pixelsWide] = macbethResolutions[resolution];
    const int  blockSize                 = pixelsWide / cols;
    const auto pixelsHigh                = blockSize * rows;
    const auto expectedPixelCount        = pixelsWide * pixelsHigh * channelCount;
    auto       image                     = ImageT(pixelsWide * pixelsHigh * channelCount, 0);

    auto           colorIndex       = 0;
    constexpr auto gridCount        = 2U;
    auto           actualPixelCount = 0;
    for (auto r = 0; r < rows; ++r)
    {
        const auto rowStart = r * blockSize;
        const auto rowStop  = rowStart + blockSize;

        for (auto c = 0; c < cols; ++c)
        {
            const auto colStart = c * blockSize;
            const auto colStop  = colStart + blockSize;

            const auto &rgbColors            = macbethPatches[colorIndex];
            const auto &[c00, c01, c10, c11] = get_colors_for_bayer_layout<BitDepthT>(bayerLayout, rgbColors, colorIndex);

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
                    actualPixelCount += 4;
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

class TiffHandler
{
  public:
    TiffHandler(const std::filesystem::path fullFilePath)
    {
        tiffHandle = TIFFOpen(fullFilePath.c_str(), "w");
    }
    ~TiffHandler()
    {
        if (tiffHandle != nullptr)
        {
            TIFFClose(tiffHandle);
        }
    }
    TiffHandler() = delete;

    bool IsValid() { return tiffHandle != nullptr; }

    operator TIFF *() { return tiffHandle; }

  private:
    TIFF *tiffHandle = nullptr;
};

/*
  Incoming image is RGGB, or similar.
  Pixels wide should be width of image, were 4 channels make up 1 pixel
  Pixels high have the same attribute as Pixels wide.
 */
template <typename ImageT>
bool save_image_as_tiff(const ImageT &image, const std::filesystem::path fullFilePath, const std::int32_t width, const std::int32_t height)
{
    auto tiff = TiffHandler(fullFilePath);
    if (tiff.IsValid() == false) { return false; }

    // TODO: 8-bit centric code in this section has to change to allow for 16-bit values as well
    const auto adjustedWidth  = width * 2U;
    const auto adjustedHeight = height * 2U;
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, adjustedWidth);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, adjustedHeight);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1); // Greyscale
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);   // TODO: Change
    TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW); // Optional compression

    tsize_t       linebytes = adjustedWidth;                            // Width in bytes for 8-bit
    std::uint8_t *raster    = const_cast<std::uint8_t *>(image.data()); // TODO: questionable code by AI

    for (auto row = 0; row < adjustedHeight; ++row)
    {
        // Write each scanline
        if (TIFFWriteScanline(tiff, raster + (row * linebytes), row, 0) < 0)
        {
            return false;
        }
    }

    return false;
}

}; // namespace

int main(int, char **)
{
    std::cout << "Hello!\n";

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR, MacbethResolution::Screen);

    const auto [height2, width] = macbethResolutions[MacbethResolution::Screen];
    const auto height           = (width / 6U) * 4U;
    const auto result           = save_image_as_tiff<Image1D_8U>(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff",
                                                                 width, height);
    // const auto result = save_image(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff", pixelsWide, pixelsHigh);
    // std::cout << "Image save result: " << (result ? "Success\n" : "Fail\n");

    return result == true ? 0 : 1;
}
