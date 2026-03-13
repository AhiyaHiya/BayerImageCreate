#ifndef __BAYER_HPP__
#define __BAYER_HPP__

#include "Types/Types.hpp"

#include <cstdint>

constexpr auto BayerChannelCount = 4U; // RGGB

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

/*
 Uses Debayer layout to determine which color is returned
   for (0,0), (0,1), (1,0), (1,1)
*/
template <typename BitDepthT, typename C00 = BitDepthT, typename C01 = BitDepthT, typename C10 = BitDepthT, typename C11 = BitDepthT>
auto get_colors_for_bayer_layout(const DemosaicTypes bayerLayout,
                                 const BitDepthT r, const BitDepthT g, const BitDepthT b)
    -> std::tuple<C00, C01, C10, C11>
{
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
Returns interleaved image.
If Bayer format is RGGB:
Row 0 RGRGRGRGRGRGRGRG
Row 1 GBGBGBGBGBGBGBGB
*/
template <typename MacbethT, typename BitDepthT = MacbethT::bit_depth_type, typename BayerT = MacbethT::container_type>
auto convert_rgb_data_to_bayer_layout(const MacbethT &rgbData, const DemosaicTypes bayerLayout) -> std::tuple<BayerT, width_t, height_t>
{
    const auto totalPixelCount = rgbData.PixelsWide * rgbData.PixelsHigh * BayerChannelCount;

    auto       bayerImage     = BayerT(totalPixelCount, 0);
    const auto gridCount      = 2U;
    const auto bayerRowStride = rgbData.PixelsWide * gridCount;
    const auto rgbRowStride   = rgbData.PixelsWide * rgbData.ChannelCount;
    for (auto row = 0; row < rgbData.PixelsHigh; ++row)
    {
        for (auto col = 0; col < rgbData.PixelsWide; ++col)
        {
            const auto rgbOffset = (row * rgbRowStride) + (col * rgbData.ChannelCount);
            const auto r         = rgbData.RgbData[rgbOffset + 0];
            const auto g         = rgbData.RgbData[rgbOffset + 1];
            const auto b         = rgbData.RgbData[rgbOffset + 2];

            const auto &[c00, c01, c10, c11] = get_colors_for_bayer_layout<BitDepthT>(bayerLayout, r, g, b);

            const auto bayerOffset0      = (row * gridCount * bayerRowStride) + (col * gridCount);
            bayerImage[bayerOffset0 + 0] = c00;
            bayerImage[bayerOffset0 + 1] = c01;

            const auto bayerOffset1      = ((row * gridCount + 1) * bayerRowStride) + (col * gridCount);
            bayerImage[bayerOffset1 + 0] = c10;
            bayerImage[bayerOffset1 + 1] = c11;
        }
    }
    return {bayerImage, rgbData.PixelsWide * gridCount, rgbData.PixelsHigh * gridCount};
}

#endif // __BAYER_HPP__
