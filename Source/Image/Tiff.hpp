#ifndef __TIFF_HPP__
#define __TIFF_HPP__

#include <tiffio.h>

#include <cstdint>
#include <filesystem>

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

#endif // __TIFF_HPP__
