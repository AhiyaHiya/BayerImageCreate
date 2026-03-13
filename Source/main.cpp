
#include "Image/JPEG.hpp"
#include "Image/Tiff.hpp"
#include "Macbeth/Macbeth.hpp"
#include "Types/Types.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
// #include <mdspan> // LLVM 21 doesn't not have mdspan and neither does GCC 15
#include <vector>

namespace
{

}; // namespace

int main(int, char **)
{
    const auto macbethResolution = MacbethResolution::Screen;
    const auto [height, width]   = MacbethResolutions[macbethResolution];

    auto macbethImage = CreateMacbethImage<Image1D_8U>(macbethResolution);

    const auto jpegResult = save_image(macbethImage, "/home/jaimerios/Pictures/MacbethImageRGB.jpg");
    std::cout << "JPEG Image save result: " << (jpegResult ? "Success\n" : "Fail\n");

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR, macbethResolution);

    auto [bayerImage, bayerPixelsWide, bayerPixelsHigh] = convert_rgb_data_to_bayer_layout(macbethImage, DemosaicTypes::COLOR_BayerRGGB2BGR);

    const auto result = save_image_as_tiff(bayerImage, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff",
                                           bayerPixelsWide, bayerPixelsHigh);

    return result == true ? 0 : 1;
}
