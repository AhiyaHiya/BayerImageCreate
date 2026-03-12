#ifndef __BAYER_HPP__
#define __BAYER_HPP__

#include <cstdint>

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

#endif // __BAYER_HPP__
