
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
    std::cout << "Hello!\n";

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR, MacbethResolution::Screen);

    const auto [height, width] = MacbethResolutions[MacbethResolution::Screen];
    const auto result          = save_image_as_tiff<Image1D_8U>(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff",
                                                                width, height);

    auto       rgbImage   = create_macbeth_colorchecker_data<Image1D_8U>();
    const auto jpegResult = save_image(rgbImage, "/home/jaimerios/Pictures/MacbethImageRGB.jpg", width, height);
    std::cout << "JPEG Image save result: " << (jpegResult ? "Success\n" : "Fail\n");

    return result == true ? 0 : 1;
}
