
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

    auto       rgbImage     = create_macbeth_colorchecker_data<Image1D_8U>(macbethResolution);
    auto       macbethImage = CreateMacbethImage<Image1D_8U>(macbethResolution);
    const auto jpegResult   = save_image(macbethImage, "/home/jaimerios/Pictures/MacbethImageRGB.jpg");
    std::cout << "JPEG Image save result: " << (jpegResult ? "Success\n" : "Fail\n");

    auto image = create_macbeth_colorchecker_image<Image1D_8U>(DemosaicTypes::COLOR_BayerRGGB2BGR, macbethResolution);

    const auto result = save_image_as_tiff<Image1D_8U>(image, "/home/jaimerios/Pictures/MacbethImageRGGB_RAW.tiff",
                                                       width, height);

    return result == true ? 0 : 1;
}
