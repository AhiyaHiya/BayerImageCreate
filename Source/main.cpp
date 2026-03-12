#include "Image/Bayer.hpp"
#include "Macbeth/Macbeth.hpp"
#include "Types/Types.hpp"

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
