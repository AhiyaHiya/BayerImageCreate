#ifndef __TIFF_HPP__
#define __TIFF_HPP__

#include "Types/Types.hpp"

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
bool save_image_as_tiff(const ImageT &bayerImage, const std::filesystem::path fullFilePath,
                        const width_t bayerPixelsWide, const height_t bayerPixelsHigh)
{
    auto tiff = TiffHandler(fullFilePath);
    if (tiff.IsValid() == false) { return false; }

    // TODO: 8-bit centric code in this section has to change to allow for 16-bit values as well
    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, bayerPixelsWide);
    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, bayerPixelsHigh);
    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 1); // Greyscale
    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);   // TODO: Change
    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1); // Often recommended

    for (auto row = 0; row < bayerPixelsHigh; ++row)
    {
        void      *scanline = (void *)&bayerImage.data()[row * bayerPixelsWide];
        const auto result   = TIFFWriteScanline(tiff, scanline, row, 0);
        if (result < 0)
        {
            return false;
        }
    }

    return false;
}

#endif // __TIFF_HPP__
